#include "infrastructure/crypto/tracer_exchange_service_internal.hpp"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <optional>
#include <sstream>
#include <stdexcept>

import tracer.core.infrastructure.crypto.exchange;

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

  return (fs::path(exchange_pkg::kPayloadRoot) / std::to_string(file_info.year) /
          file_info.file_name)
      .generic_string();
}

auto CollectInputPayloadFiles(const fs::path& input_root)
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
      throw std::runtime_error("Duplicate month TXT inputs detected for " +
                               month_info.month_key + ": " +
                               it->second.string() + " | " +
                               entry.path().string());
    }
    payload_files.push_back(InputPayloadFile{
        .source_path = entry.path(),
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

auto ValidateInputForExport(app_workflow::IWorkflowHandler& workflow_handler,
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
    ValidateInputForExport(workflow_handler, date_check_mode,
                           payload_file.source_path);
  }
}

auto BuildManifestEntry(const TracerExchangeManifest& manifest)
    -> TracerExchangePackageEntry {
  TracerExchangePackageEntry entry{};
  entry.relative_path = std::string(exchange_pkg::kManifestPath);
  const std::string text = BuildManifestText(manifest);
  entry.data = CanonicalizePackageTextBytes(text, exchange_pkg::kManifestPath);
  entry.entry_flags = exchange_pkg::kStandardEntryFlags;
  return entry;
}

auto BuildFileEntry(std::string_view relative_path, const fs::path& source_path)
    -> TracerExchangePackageEntry {
  EnsureRegularFileExists(source_path, "Tracer exchange source file");
  TracerExchangePackageEntry entry{};
  entry.relative_path = std::string(relative_path);
  const auto source_bytes = ReadFileBytes(source_path);
  entry.data = IsCanonicalTextPackagePath(relative_path)
                   ? CanonicalizePackageTextBytes(source_bytes,
                                                  source_path.string())
                   : source_bytes;
  entry.entry_flags = exchange_pkg::kStandardEntryFlags;
  return entry;
}

}  // namespace

auto TracerExchangeService::RunExport(
    const app_dto::TracerExchangeExportRequest& request)
    -> app_dto::TracerExchangeExportResult {
  if (request.input_text_root_path.empty() || request.requested_output_path.empty()) {
    throw std::invalid_argument("input/output paths are required.");
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

  const fs::path input_path = fs::absolute(request.input_text_root_path);
  if (!fs::exists(input_path) || !fs::is_directory(input_path)) {
    throw std::invalid_argument(
        "Encrypt input path must be an existing directory: " +
        input_path.string());
  }

  const std::vector<InputPayloadFile> payload_files =
      CollectInputPayloadFiles(input_path);
  if (payload_files.empty()) {
    throw std::invalid_argument(
        "Encrypt input directory must contain at least one .txt file: " +
        input_path.string());
  }

  const ActiveConverterConfigPaths config_paths =
      ResolveActiveConverterConfigPaths(request.active_converter_main_config_path);
  EnsureActiveConverterConfigExists(config_paths);
  ValidateInputPayloadsForExport(workflow_handler_, request.date_check_mode,
                                 payload_files);

  const fs::path resolved_output = ResolveEncryptOutputPath(
      input_path, fs::absolute(request.requested_output_path));
  EnsureParentDirectory(resolved_output);

  const fs::path staging_dir = BuildScopedStagingDir(
      resolved_output.parent_path(), "encrypt", resolved_output.stem().string());
  const fs::path package_path = staging_dir / "exchange.ttpkg";

  std::error_code io_error;
  fs::create_directories(staging_dir, io_error);
  if (io_error) {
    throw std::runtime_error("Failed to create tracer exchange staging dir: " +
                             staging_dir.string() + " | " + io_error.message());
  }

  try {
    TracerExchangeManifest manifest{};
    manifest.producer_platform = request.producer_platform;
    manifest.producer_app = request.producer_app;
    manifest.created_at_utc = CurrentUtcTimestampRfc3339();
    manifest.source_root_name = input_path.filename().string();
    if (manifest.source_root_name.empty()) {
      manifest.source_root_name = "text_root";
    }
    manifest.payload_files.reserve(payload_files.size());
    for (const auto& payload_file : payload_files) {
      manifest.payload_files.push_back(payload_file.relative_package_path);
    }

    std::vector<TracerExchangePackageEntry> entries;
    entries.reserve(exchange_pkg::kRequiredPackagePaths.size() +
                    payload_files.size());
    entries.push_back(BuildManifestEntry(manifest));
    entries.push_back(
        BuildFileEntry(exchange_pkg::kConverterMainPath, config_paths.main_config_path));
    entries.push_back(
        BuildFileEntry(exchange_pkg::kAliasMappingPath, config_paths.alias_mapping_path));
    entries.push_back(
        BuildFileEntry(exchange_pkg::kDurationRulesPath, config_paths.duration_rules_path));
    for (const auto& payload_file : payload_files) {
      entries.push_back(
          BuildFileEntry(payload_file.relative_package_path, payload_file.source_path));
    }

    const std::vector<std::uint8_t> package_bytes = EncodePackageBytes(entries);
    WriteFileBytes(package_path, package_bytes);

    EnsureCryptoResultOk(
        file_crypto::EncryptFile(
            package_path, resolved_output, request.passphrase,
            BuildCryptoOptions(request.security_level, request.progress_observer)),
        "Encrypt", input_path);
  } catch (...) {
    RemoveDirectoryBestEffort(staging_dir);
    throw;
  }

  RemoveDirectoryBestEffort(staging_dir);
  return {
      .ok = true,
      .resolved_output_tracer_path = resolved_output,
      .source_root_name = input_path.filename().empty()
                              ? std::string("text_root")
                              : input_path.filename().string(),
      .payload_file_count = static_cast<std::uint64_t>(payload_files.size()),
      .converter_file_count = 3,
      .manifest_included = true,
      .error_message = "",
  };
}

}  // namespace tracer_core::infrastructure::crypto::tracer_exchange_internal
