#include "infra/exchange/tracer_exchange_service_internal.hpp"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <map>
#include <optional>
#include <sstream>
#include <stdexcept>

import tracer.core.infrastructure.exchange;

namespace tracer_core::infrastructure::crypto::tracer_exchange_internal {

namespace {

using DecodedTracerExchangePackage = exchange_pkg::DecodedTracerExchangePackage;
using TracerExchangeManifest = exchange_pkg::TracerExchangeManifest;
using TracerExchangePackageEntry = exchange_pkg::TracerExchangePackageEntry;
using exchange_pkg::BuildManifestText;
using exchange_pkg::EncodePackageBytes;

auto ResolveEncryptOutputPath(const fs::path& input_path,
                              const fs::path& output_arg) -> fs::path {
  fs::path output_path = output_arg;
  if (fs::exists(output_path) && fs::is_directory(output_path)) {
    output_path /= input_path.filename();
  }
  output_path.replace_extension(".tracer");
  return output_path;
}

auto CurrentUtcTimestampRfc3339() -> std::string {
  const auto now = std::chrono::system_clock::now();
  const std::time_t raw_time = std::chrono::system_clock::to_time_t(now);
  std::tm utc_time{};
#if defined(_WIN32)
  gmtime_s(&utc_time, &raw_time);
#else
  gmtime_r(&raw_time, &utc_time);
#endif
  std::ostringstream stream;
  stream << std::put_time(&utc_time, "%Y-%m-%dT%H:%M:%SZ");
  return stream.str();
}

auto ResolvePayloadPackagePath(const fs::path& source_path) -> std::string {
  const ParsedMonthInfo file_info = ParseMonthInfoFromFileName(
      source_path.filename().string(), source_path.string());
  const ParsedMonthInfo header_info = ParseMonthInfoFromCanonicalText(
      ReadFileBytes(source_path), source_path.string());
  if (file_info.month_key != header_info.month_key) {
    throw std::runtime_error(
        "TXT file name must match canonical month headers yYYYY + mMM: " +
        source_path.string());
  }

  return (fs::path(exchange_pkg::kPayloadRoot) /
          std::to_string(file_info.year) / file_info.file_name)
      .generic_string();
}

auto ResolvePayloadPackagePath(const ParsedMonthInfo& header_info)
    -> std::string {
  return (fs::path(exchange_pkg::kPayloadRoot) /
          std::to_string(header_info.year) / header_info.file_name)
      .generic_string();
}

auto CollectInputPayloadFilesFromRoot(const fs::path& input_root)
    -> std::vector<InputPayloadFile> {
  std::vector<InputPayloadFile> payload_files;
  std::map<std::string, fs::path> path_by_month;
  for (const auto& entry : fs::recursive_directory_iterator(input_root)) {
    if (!entry.is_regular_file()) {
      continue;
    }
    if (!HasExtensionCaseInsensitive(entry.path(), ".txt")) {
      continue;
    }
    const std::string relative_package_path =
        ResolvePayloadPackagePath(entry.path());
    const ParsedMonthInfo month_info = ParseMonthInfoFromFileName(
        entry.path().filename().string(), entry.path().string());
    if (const auto [it, inserted] =
            path_by_month.emplace(month_info.month_key, entry.path());
        !inserted) {
      throw std::runtime_error(
          "Duplicate month TXT inputs detected for " + month_info.month_key +
          ": " + it->second.string() + " | " + entry.path().string());
    }
    payload_files.push_back(InputPayloadFile{
        .source_path = entry.path(),
        .source_label = entry.path().string(),
        .relative_package_path = relative_package_path,
        .month_key = month_info.month_key,
        .year = month_info.year,
        .month = month_info.month,
    });
  }

  std::sort(payload_files.begin(), payload_files.end(),
            [](const InputPayloadFile& lhs, const InputPayloadFile& rhs) {
              return lhs.relative_package_path < rhs.relative_package_path;
            });
  return payload_files;
}

auto CollectInputPayloadFilesFromPayloads(
    const std::vector<app_dto::TracerExchangeTextPayloadItem>& payload_items)
    -> std::vector<InputPayloadFile> {
  std::vector<InputPayloadFile> payload_files;
  std::map<std::string, std::string> label_by_month;
  payload_files.reserve(payload_items.size());
  for (const auto& payload_item : payload_items) {
    const std::string source_label = payload_item.relative_path_hint.empty()
                                         ? std::string("(payload_item)")
                                         : payload_item.relative_path_hint;
    const std::vector<std::uint8_t> content_bytes(payload_item.content.begin(),
                                                  payload_item.content.end());
    const ParsedMonthInfo header_info =
        ParseMonthInfoFromCanonicalText(content_bytes, source_label);
    const std::string relative_package_path =
        ResolvePayloadPackagePath(header_info);
    if (!payload_item.relative_path_hint.empty()) {
      const std::string normalized_hint =
          fs::path(payload_item.relative_path_hint).generic_string();
      const std::string expected_hint =
          (fs::path(std::to_string(header_info.year)) / header_info.file_name)
              .generic_string();
      if (normalized_hint != expected_hint) {
        throw std::runtime_error(
            "TXT payload hint must match canonical month headers yYYYY + mMM: " +
            payload_item.relative_path_hint + " -> expected " + expected_hint);
      }
    }
    if (const auto [it, inserted] =
            label_by_month.emplace(header_info.month_key, source_label);
        !inserted) {
      throw std::runtime_error(
          "Duplicate month TXT payloads detected for " + header_info.month_key +
          ": " + it->second + " | " + source_label);
    }

    payload_files.push_back(InputPayloadFile{
        .source_label = source_label,
        .content_bytes = content_bytes,
        .relative_package_path = relative_package_path,
        .month_key = header_info.month_key,
        .year = header_info.year,
        .month = header_info.month,
    });
  }

  std::sort(payload_files.begin(), payload_files.end(),
            [](const InputPayloadFile& lhs, const InputPayloadFile& rhs) {
              return lhs.relative_package_path < rhs.relative_package_path;
            });
  return payload_files;
}

auto ValidateInputForExport(
    app_workflow::IWorkflowHandler& workflow_handler,
    tracer::core::domain::types::DateCheckMode date_check_mode,
    const fs::path& input_path) -> void {
  const std::string input = input_path.string();
  workflow_handler.RunValidateStructure(input);
  workflow_handler.RunValidateLogic(input, date_check_mode);
}

auto ValidateInputPayloadsForExport(
    app_workflow::IWorkflowHandler& workflow_handler,
    tracer::core::domain::types::DateCheckMode date_check_mode,
    const std::vector<InputPayloadFile>& payload_files) -> void {
  for (const auto& payload_file : payload_files) {
    if (payload_file.source_path.empty()) {
      continue;
    }
    ValidateInputForExport(workflow_handler, date_check_mode,
                           payload_file.source_path);
  }
}

auto BuildManifestEntry(const TracerExchangeManifest& manifest)
    -> TracerExchangePackageEntry {
  TracerExchangePackageEntry entry{};
  entry.relative_path = std::string(exchange_pkg::kManifestPath);
  const std::string kText = BuildManifestText(manifest);
  entry.data = CanonicalizePackageTextBytes(kText, exchange_pkg::kManifestPath);
  entry.entry_flags = exchange_pkg::kStandardEntryFlags;
  return entry;
}

auto BuildFileEntry(std::string_view relative_path, const fs::path& source_path)
    -> TracerExchangePackageEntry {
  EnsureRegularFileExists(source_path, "Tracer exchange source file");
  TracerExchangePackageEntry entry{};
  entry.relative_path = std::string(relative_path);
  const auto kSourceBytes = ReadFileBytes(source_path);
  entry.data =
      IsCanonicalTextPackagePath(relative_path)
          ? CanonicalizePackageTextBytes(kSourceBytes, source_path.string())
          : kSourceBytes;
  entry.entry_flags = exchange_pkg::kStandardEntryFlags;
  return entry;
}

auto BuildPayloadEntry(const InputPayloadFile& payload_file)
    -> TracerExchangePackageEntry {
  if (!payload_file.source_path.empty()) {
    return BuildFileEntry(payload_file.relative_package_path,
                          payload_file.source_path);
  }

  TracerExchangePackageEntry entry{};
  entry.relative_path = payload_file.relative_package_path;
  entry.data = CanonicalizePackageTextBytes(payload_file.content_bytes,
                                            payload_file.source_label);
  entry.entry_flags = exchange_pkg::kStandardEntryFlags;
  return entry;
}

}  // namespace

auto TracerExchangeService::RunExport(
    const app_dto::TracerExchangeExportRequest& request)
    -> app_dto::TracerExchangeExportResult {
  const bool has_input_root = !request.input_text_root_path.empty();
  const bool has_input_payloads = !request.input_text_payloads.empty();
  if (has_input_root == has_input_payloads) {
    throw std::invalid_argument(
        "Exactly one export input source is required.");
  }
  const bool has_output_path = !request.requested_output_path.empty();
  const bool has_output_writer =
      static_cast<bool>(request.encrypted_output_writer);
  if (has_output_path == has_output_writer) {
    throw std::invalid_argument(
        "Exactly one export output target is required.");
  }
  if (request.active_converter_main_config_path.empty()) {
    throw std::invalid_argument(
        "active_converter_main_config_path must not be empty.");
  }
  if (request.passphrase.empty()) {
    throw std::invalid_argument("Passphrase must not be empty.");
  }
  if (request.producer_platform.empty() || request.producer_app.empty()) {
    throw std::invalid_argument(
        "producer_platform/producer_app must not be empty.");
  }

  const fs::path kInputPath =
      has_input_root ? fs::absolute(request.input_text_root_path) : fs::path{};
  if (has_input_root && (!fs::exists(kInputPath) || !fs::is_directory(kInputPath))) {
    throw std::invalid_argument(
        "Encrypt input path must be an existing directory: " +
        kInputPath.string());
  }

  const std::vector<InputPayloadFile> kPayloadFiles =
      has_input_root
          ? CollectInputPayloadFilesFromRoot(kInputPath)
          : CollectInputPayloadFilesFromPayloads(request.input_text_payloads);
  if (kPayloadFiles.empty()) {
    throw std::invalid_argument("Export input must contain at least one TXT payload.");
  }

  const ActiveConverterConfigPaths kConfigPaths =
      ResolveActiveConverterConfigPaths(
          request.active_converter_main_config_path);
  EnsureActiveConverterConfigExists(kConfigPaths);
  ValidateInputPayloadsForExport(workflow_handler_, request.date_check_mode,
                                 kPayloadFiles);
  const std::string kSourceRootName =
      !request.logical_source_root_name.empty()
          ? request.logical_source_root_name
          : (has_input_root ? (kInputPath.filename().empty()
                                   ? std::string("text_root")
                                   : kInputPath.filename().string())
                            : std::string("data"));
  const fs::path kResolvedOutput =
      has_output_path
          ? ResolveEncryptOutputPath(kInputPath.empty() ? fs::path(kSourceRootName)
                                                        : kInputPath,
                                     fs::absolute(request.requested_output_path))
          : fs::path("android_export_sink") /
                (request.output_display_name.empty()
                     ? (kSourceRootName + ".tracer")
                     : request.output_display_name);
  if (has_output_path) {
    EnsureParentDirectory(kResolvedOutput);
  }

  TracerExchangeManifest manifest{};
  manifest.producer_platform = request.producer_platform;
  manifest.producer_app = request.producer_app;
  manifest.created_at_utc = CurrentUtcTimestampRfc3339();
  manifest.source_root_name = kSourceRootName;
  manifest.payload_files.reserve(kPayloadFiles.size());
  for (const auto& payload_file : kPayloadFiles) {
    manifest.payload_files.push_back(payload_file.relative_package_path);
  }

  std::vector<TracerExchangePackageEntry> entries;
  entries.reserve(exchange_pkg::kRequiredPackagePaths.size() +
                  kPayloadFiles.size());
  entries.push_back(BuildManifestEntry(manifest));
  entries.push_back(BuildFileEntry(exchange_pkg::kConverterMainPath,
                                   kConfigPaths.main_config_path));
  entries.push_back(BuildFileEntry(exchange_pkg::kAliasMappingPath,
                                   kConfigPaths.alias_mapping_path));
  entries.push_back(BuildFileEntry(exchange_pkg::kDurationRulesPath,
                                   kConfigPaths.duration_rules_path));
  for (const auto& payload_file : kPayloadFiles) {
    entries.push_back(BuildPayloadEntry(payload_file));
  }

  const std::vector<std::uint8_t> kPackageBytes = EncodePackageBytes(entries);
  const file_crypto::FileCryptoPathContext kPathContext{
      .input_root_path = has_input_root ? kInputPath : fs::path(kSourceRootName),
      .output_root_path = kResolvedOutput.parent_path(),
      .current_input_path = has_input_root
                                ? kInputPath
                                : fs::path(kSourceRootName) / "payload.ttpkg",
      .current_output_path = kResolvedOutput,
  };
  const auto kCryptoOptions =
      BuildCryptoOptions(request.security_level, request.progress_observer);
  const auto encrypt_result =
      has_output_path
          ? file_crypto::EncryptBytesToFile(kPackageBytes, kResolvedOutput,
                                            request.passphrase, kPathContext,
                                            kCryptoOptions)
          : file_crypto::EncryptBytesToWriter(
                kPackageBytes,
                [&request](std::span<const std::uint8_t> ciphertext_bytes)
                    -> file_crypto::FileCryptoResult {
                  std::string error_message;
                  if (request.encrypted_output_writer(ciphertext_bytes,
                                                      error_message)) {
                    return {};
                  }
                  return {
                      .error = file_crypto::FileCryptoError::kOutputWriteFailed,
                      .error_code = std::string(
                          file_crypto::ToErrorCode(
                              file_crypto::FileCryptoError::kOutputWriteFailed)),
                      .error_message = error_message.empty()
                                           ? std::string(
                                                 "Failed to write encrypted "
                                                 "exchange output.")
                                           : error_message,
                  };
                },
                request.passphrase, kPathContext, kCryptoOptions);
  EnsureCryptoResultOk(encrypt_result, "Encrypt",
                       has_input_root ? kInputPath
                                      : fs::path(kSourceRootName));
  return {
      .ok = true,
      .resolved_output_tracer_path = kResolvedOutput,
      .source_root_name = kSourceRootName,
      .payload_file_count = static_cast<std::uint64_t>(kPayloadFiles.size()),
      .converter_file_count = 3,
      .manifest_included = true,
      .error_message = "",
  };
}

}  // namespace tracer_core::infrastructure::crypto::tracer_exchange_internal
