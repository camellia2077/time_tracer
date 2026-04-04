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

struct ResolveEncryptOutputPathRequest {
  fs::path input_path;
  fs::path output_arg;
};

auto ResolveEncryptOutputPath(const ResolveEncryptOutputPathRequest& request)
    -> fs::path {
  fs::path output_path = request.output_arg;
  if (fs::exists(output_path) && fs::is_directory(output_path)) {
    output_path /= request.input_path.filename();
  }
  output_path.replace_extension(".tracer");
  return output_path;
}

auto CurrentUtcTimestampRfc3339() -> std::string {
  const auto kNow = std::chrono::system_clock::now();
  const std::time_t kRawTime = std::chrono::system_clock::to_time_t(kNow);
  std::tm utc_time{};
#if defined(_WIN32)
  gmtime_s(&utc_time, &kRawTime);
#else
  gmtime_r(&raw_time, &utc_time);
#endif
  std::ostringstream stream;
  stream << std::put_time(&utc_time, "%Y-%m-%dT%H:%M:%SZ");
  return stream.str();
}

auto ResolvePayloadPackagePath(const fs::path& source_path) -> std::string {
  const ParsedMonthInfo kFileInfo = ParseMonthInfoFromFileName(
      source_path.filename().string(), source_path.string());
  const ParsedMonthInfo kHeaderInfo = ParseMonthInfoFromCanonicalText(
      ReadFileBytes(source_path), source_path.string());
  if (kFileInfo.month_key != kHeaderInfo.month_key) {
    throw std::runtime_error(
        "TXT file name must match canonical month headers yYYYY + mMM: " +
        source_path.string());
  }

  return (fs::path(exchange_pkg::kPayloadRoot) /
          std::to_string(kFileInfo.year) / kFileInfo.file_name)
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
    const std::string kRelativePackagePath =
        ResolvePayloadPackagePath(entry.path());
    const ParsedMonthInfo kMonthInfo = ParseMonthInfoFromFileName(
        entry.path().filename().string(), entry.path().string());
    if (const auto [it, inserted] =
            path_by_month.emplace(kMonthInfo.month_key, entry.path());
        !inserted) {
      throw std::runtime_error(
          "Duplicate month TXT inputs detected for " + kMonthInfo.month_key +
          ": " + it->second.string() + " | " + entry.path().string());
    }
    payload_files.push_back(InputPayloadFile{
        .source_path = entry.path(),
        .source_label = entry.path().string(),
        .relative_package_path = kRelativePackagePath,
        .month_key = kMonthInfo.month_key,
        .year = kMonthInfo.year,
        .month = kMonthInfo.month,
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
    const std::string kSourceLabel = payload_item.relative_path_hint.empty()
                                         ? std::string("(payload_item)")
                                         : payload_item.relative_path_hint;
    const std::vector<std::uint8_t> kContentBytes(payload_item.content.begin(),
                                                  payload_item.content.end());
    const ParsedMonthInfo kHeaderInfo =
        ParseMonthInfoFromCanonicalText(kContentBytes, kSourceLabel);
    const std::string kRelativePackagePath =
        ResolvePayloadPackagePath(kHeaderInfo);
    if (!payload_item.relative_path_hint.empty()) {
      const std::string kNormalizedHint =
          fs::path(payload_item.relative_path_hint).generic_string();
      const std::string kExpectedHint =
          (fs::path(std::to_string(kHeaderInfo.year)) / kHeaderInfo.file_name)
              .generic_string();
      if (kNormalizedHint != kExpectedHint) {
        throw std::runtime_error(
            "TXT payload hint must match canonical month headers yYYYY + mMM: " +
            payload_item.relative_path_hint + " -> expected " + kExpectedHint);
      }
    }
    if (const auto [it, inserted] =
            label_by_month.emplace(kHeaderInfo.month_key, kSourceLabel);
        !inserted) {
      throw std::runtime_error(
          "Duplicate month TXT payloads detected for " + kHeaderInfo.month_key +
          ": " + it->second + " | " + kSourceLabel);
    }

    payload_files.push_back(InputPayloadFile{
        .source_label = kSourceLabel,
        .content_bytes = kContentBytes,
        .relative_package_path = kRelativePackagePath,
        .month_key = kHeaderInfo.month_key,
        .year = kHeaderInfo.year,
        .month = kHeaderInfo.month,
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
  const std::string kInput = input_path.string();
  workflow_handler.RunValidateStructure(kInput);
  workflow_handler.RunValidateLogic(kInput, date_check_mode);
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
  const bool kHasInputRoot = !request.input_text_root_path.empty();
  const bool kHasInputPayloads = !request.input_text_payloads.empty();
  if (kHasInputRoot == kHasInputPayloads) {
    throw std::invalid_argument(
        "Exactly one export input source is required.");
  }
  const bool kHasOutputPath = !request.requested_output_path.empty();
  const bool kHasOutputWriter =
      static_cast<bool>(request.encrypted_output_writer);
  if (kHasOutputPath == kHasOutputWriter) {
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
      kHasInputRoot ? fs::absolute(request.input_text_root_path) : fs::path{};
  if (kHasInputRoot && (!fs::exists(kInputPath) || !fs::is_directory(kInputPath))) {
    throw std::invalid_argument(
        "Encrypt input path must be an existing directory: " +
        kInputPath.string());
  }

  const std::vector<InputPayloadFile> kPayloadFiles =
      kHasInputRoot
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
          : (kHasInputRoot ? (kInputPath.filename().empty()
                                   ? std::string("text_root")
                                   : kInputPath.filename().string())
                            : std::string("data"));
  const fs::path kResolvedOutput =
      kHasOutputPath
          ? ResolveEncryptOutputPath({
                .input_path =
                    kInputPath.empty() ? fs::path(kSourceRootName) : kInputPath,
                .output_arg = fs::absolute(request.requested_output_path),
            })
          : fs::path("android_export_sink") /
                (request.output_display_name.empty()
                     ? (kSourceRootName + ".tracer")
                     : request.output_display_name);
  if (kHasOutputPath) {
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
      .input_root_path = kHasInputRoot ? kInputPath : fs::path(kSourceRootName),
      .output_root_path = kResolvedOutput.parent_path(),
      .current_input_path = kHasInputRoot
                                ? kInputPath
                                : fs::path(kSourceRootName) / "payload.ttpkg",
      .current_output_path = kResolvedOutput,
  };
  const auto kCryptoOptions =
      BuildCryptoOptions(request.security_level, request.progress_observer);
  const auto kEncryptResult =
      kHasOutputPath
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
  EnsureCryptoResultOk(kEncryptResult, "Encrypt",
                       kHasInputRoot ? kInputPath
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
