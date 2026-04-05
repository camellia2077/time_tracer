#include "infra/exchange/tracer_exchange_service_internal.hpp"
#include "infra/exchange/tracer_exchange_service_export_support.hpp"

#include <optional>
#include <stdexcept>

import tracer.core.infrastructure.exchange;

namespace tracer_core::infrastructure::crypto::tracer_exchange_internal {

namespace {

using DecodedTracerExchangePackage = exchange_pkg::DecodedTracerExchangePackage;
using TracerExchangeManifest = exchange_pkg::TracerExchangeManifest;
using TracerExchangePackageEntry = exchange_pkg::TracerExchangePackageEntry;
using exchange_pkg::BuildManifestText;
using exchange_pkg::EncodePackageBytes;

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
