#include "infra/exchange/tracer_exchange_service_internal.hpp"
#include "infra/exchange/tracer_exchange_service_export_support.hpp"

#include <algorithm>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <utility>

import tracer.core.infrastructure.exchange;

namespace tracer_core::infrastructure::crypto::tracer_exchange_internal {

namespace {

using TracerExchangeManifest = exchange_pkg::TracerExchangeManifest;
using TracerExchangePackageEntry = exchange_pkg::TracerExchangePackageEntry;
using exchange_pkg::BuildManifestText;
using exchange_pkg::EncodePackageBytes;

auto UsesExistingProtection(
    const app_dto::TracerExchangeProtectionOptions& protection) -> bool {
  return protection.compression ==
             app_dto::TracerExchangeCompressionMode::kExisting &&
         protection.encryption ==
             app_dto::TracerExchangeEncryptionMode::kExisting;
}

auto UsesNoProtection(
    const app_dto::TracerExchangeProtectionOptions& protection) -> bool {
  return protection.compression ==
             app_dto::TracerExchangeCompressionMode::kNone &&
         protection.encryption == app_dto::TracerExchangeEncryptionMode::kNone;
}

auto EnsureSupportedProtection(
    const app_dto::TracerExchangeProtectionOptions& protection) -> void {
  if (!UsesExistingProtection(protection) && !UsesNoProtection(protection)) {
    throw std::invalid_argument(
        "Compression and encryption must both select None or both select "
        "the existing implementation.");
  }
}

auto ToPackageManifest(const app_dto::TracerExchangeContentManifest& manifest)
    -> TracerExchangeManifest {
  return {
      .package_type = manifest.package_type,
      .package_version = manifest.package_version,
      .producer_platform = manifest.producer_platform,
      .producer_app = manifest.producer_app,
      .created_at_utc = manifest.created_at_utc,
      .source_root_name = manifest.source_root_name,
      .config_root = manifest.config_root,
      .config_files = manifest.config_files,
      .payload_root = manifest.payload_root,
      .payload_files = manifest.payload_files,
  };
}

auto ToPackageEntry(const app_dto::TracerExchangeContentEntry& content_entry)
    -> TracerExchangePackageEntry {
  TracerExchangePackageEntry entry{};
  entry.relative_path = content_entry.relative_path;
  entry.data = content_entry.data;
  entry.entry_flags =
      static_cast<std::uint16_t>(content_entry.required ? 0x0001U : 0U) |
      static_cast<std::uint16_t>(content_entry.text ? 0x0002U : 0U);
  return entry;
}

auto BuildManifestEntry(const app_dto::TracerExchangeContentManifest& manifest)
    -> app_dto::TracerExchangeContentEntry {
  const TracerExchangeManifest kPackageManifest = ToPackageManifest(manifest);
  app_dto::TracerExchangeContentEntry entry{};
  entry.relative_path = std::string(exchange_pkg::kManifestPath);
  entry.data = CanonicalizePackageTextBytes(BuildManifestText(kPackageManifest),
                                            exchange_pkg::kManifestPath);
  entry.kind = app_dto::TracerExchangeContentEntryKind::kManifest;
  entry.required = true;
  entry.text = true;
  return entry;
}

auto BuildConfigEntries(const fs::path& config_user_root,
                        std::vector<std::string>& config_files)
    -> std::vector<app_dto::TracerExchangeContentEntry> {
  std::vector<app_dto::TracerExchangeContentEntry> entries;
  if (!fs::exists(config_user_root) || !fs::is_directory(config_user_root)) {
    throw std::invalid_argument(
        "config_user_root_path must be an existing directory: " +
        config_user_root.string());
  }

  for (const auto& filesystem_entry :
       fs::recursive_directory_iterator(config_user_root)) {
    if (!filesystem_entry.is_regular_file()) {
      continue;
    }

    const fs::path kRelativePath =
        filesystem_entry.path().lexically_relative(config_user_root);
    if (kRelativePath.empty() || kRelativePath == filesystem_entry.path() ||
        kRelativePath.string().starts_with("..")) {
      throw std::runtime_error("Config file must stay under config/user: " +
                               filesystem_entry.path().string());
    }

    const std::string kPackagePath =
        (fs::path("config/user") / kRelativePath).generic_string();
    const auto kData = ReadFileBytes(filesystem_entry.path());
    const bool kIsText = IsCanonicalTextPackagePath(kPackagePath);
    entries.push_back({
        .relative_path = kPackagePath,
        .data =
            kIsText ? CanonicalizePackageTextBytes(kData, kPackagePath) : kData,
        .kind = app_dto::TracerExchangeContentEntryKind::kConfig,
        .required = true,
        .text = kIsText,
    });
    config_files.push_back(kPackagePath);
  }

  std::sort(entries.begin(), entries.end(),
            [](const auto& left, const auto& right) {
              return left.relative_path < right.relative_path;
            });
  std::sort(config_files.begin(), config_files.end());
  if (entries.empty()) {
    throw std::invalid_argument(
        "config_user_root_path must contain at least one regular file.");
  }
  return entries;
}

auto BuildPayloadEntry(const InputPayloadFile& payload_file)
    -> app_dto::TracerExchangeContentEntry {
  app_dto::TracerExchangeContentEntry entry{};
  entry.relative_path = payload_file.relative_package_path;
  entry.data = payload_file.source_path.empty()
                   ? CanonicalizePackageTextBytes(payload_file.content_bytes,
                                                  payload_file.source_label)
                   : CanonicalizePackageTextBytes(
                         ReadFileBytes(payload_file.source_path),
                         payload_file.source_path.string());
  entry.kind = app_dto::TracerExchangeContentEntryKind::kPayload;
  entry.required = true;
  entry.text = true;
  return entry;
}

auto ResolveUnprotectedOutputName(std::string_view source_root_name,
                                  std::string_view requested_name) -> fs::path {
  fs::path output = requested_name.empty()
                        ? fs::path(std::string(source_root_name) + ".ttpkg")
                        : fs::path(std::string(requested_name));
  output.replace_extension(".ttpkg");
  return output;
}

auto ResolveUnprotectedOutputPath(const fs::path& input_path,
                                  std::string_view source_root_name,
                                  const fs::path& output_arg) -> fs::path {
  fs::path output = output_arg;
  if (fs::exists(output) && fs::is_directory(output)) {
    output /= input_path.empty() ? fs::path(std::string(source_root_name))
                                 : input_path.filename();
  }
  output.replace_extension(".ttpkg");
  return output;
}

auto ProtectAndWriteEncodedPackage(
    std::span<const std::uint8_t> package_bytes,
    const app_dto::TracerExchangeExportRequest& request,
    const fs::path& resolved_output) -> void {
  const bool kHasOutputPath = !request.requested_output_path.empty();
  if (UsesNoProtection(request.protection)) {
    if (kHasOutputPath) {
      WriteFileBytes(resolved_output, package_bytes);
      return;
    }
    std::string error_message;
    if (request.encrypted_output_writer(package_bytes, error_message)) {
      return;
    }
    throw std::runtime_error(error_message.empty()
                                 ? "Failed to write exchange output."
                                 : error_message);
  }
  const auto kPackage = exchange_pkg::DecodePackageBytes(package_bytes);
  const auto kEmitProgress = [&](std::string_view phase, std::size_t done,
                                 std::size_t total) {
    if (!request.progress_observer) {
      return;
    }
    app_dto::TracerExchangeProgressSnapshot snapshot{};
    snapshot.output_root_path = resolved_output.parent_path();
    snapshot.current_output_path = resolved_output;
    snapshot.current_item = resolved_output.filename().string();
    snapshot.current_group_label = "zip_aes_exchange";
    snapshot.phase_index = done == total ? 3U : 1U;
    snapshot.phase_count = 3U;
    snapshot.done_count = done;
    snapshot.total_count = total;
    // ZIP encoding is an atomic package operation. Export exposes only its
    // overall logical progress; there is no meaningful current-file stream.
    snapshot.overall_done_bytes = done;
    snapshot.overall_total_bytes = total;
    snapshot.is_encrypt_operation = true;
    snapshot.phase = std::string(phase);
    if (request.progress_observer(snapshot) ==
        app_dto::TracerExchangeProgressControl::kCancel) {
      throw std::runtime_error("ZIP exchange export was cancelled.");
    }
  };
  kEmitProgress("zip_prepare", 0U, kPackage.entries.size());
  const auto kZipBytes =
      exchange_pkg::EncodeZipBytes(kPackage.entries, request.passphrase);
  kEmitProgress("zip_aes_encrypt", kPackage.entries.size(),
                kPackage.entries.size());
  if (kHasOutputPath) {
    WriteFileBytes(resolved_output, kZipBytes);
    return;
  }
  std::string error_message;
  if (!request.encrypted_output_writer(kZipBytes, error_message)) {
    throw std::runtime_error(error_message.empty()
                                 ? "Failed to write encrypted ZIP exchange "
                                   "output."
                                 : error_message);
  }
}

}  // namespace

auto TracerExchangeService::BuildExportContent(
    const app_dto::TracerExchangeContentRequest& request)
    -> app_dto::TracerExchangeContentResult {
  const bool kHasInputRoot = !request.input_text_root_path.empty();
  const bool kHasInputPayloads = !request.input_text_payloads.empty();
  if (kHasInputRoot == kHasInputPayloads) {
    throw std::invalid_argument(
        "Exactly one export content input source is required.");
  }
  if (request.config_user_root_path.empty()) {
    throw std::invalid_argument("config_user_root_path must not be empty.");
  }
  if (request.active_converter_main_config_path.empty()) {
    throw std::invalid_argument(
        "active_converter_main_config_path must not be empty.");
  }
  if (request.producer_platform.empty() || request.producer_app.empty()) {
    throw std::invalid_argument(
        "producer_platform/producer_app must not be empty.");
  }

  const fs::path kConfigUserRoot = fs::absolute(request.config_user_root_path);
  const fs::path kMainConfigPath =
      fs::absolute(request.active_converter_main_config_path);
  if (kMainConfigPath.parent_path() != kConfigUserRoot) {
    throw std::invalid_argument(
        "active_converter_main_config_path must be directly under "
        "config_user_root_path.");
  }
  EnsureRegularFileExists(kMainConfigPath, "Active converter main config");

  const fs::path kInputPath =
      kHasInputRoot ? fs::absolute(request.input_text_root_path) : fs::path{};
  if (kHasInputRoot &&
      (!fs::exists(kInputPath) || !fs::is_directory(kInputPath))) {
    throw std::invalid_argument(
        "Export input path must be an existing directory: " +
        kInputPath.string());
  }

  const std::vector<InputPayloadFile> kPayloadFiles =
      kHasInputRoot
          ? CollectInputPayloadFilesFromRoot(kInputPath)
          : CollectInputPayloadFilesFromPayloads(request.input_text_payloads);
  if (kPayloadFiles.empty()) {
    throw std::invalid_argument(
        "Export input must contain at least one TXT payload.");
  }
  ValidateInputPayloadsForExport(workflow_handler_, request.date_check_mode,
                                 kPayloadFiles);

  const std::string kSourceRootName =
      !request.logical_source_root_name.empty()
          ? request.logical_source_root_name
          : (kHasInputRoot ? (kInputPath.filename().empty()
                                  ? std::string("text_root")
                                  : kInputPath.filename().string())
                           : std::string("data"));

  app_dto::TracerExchangeExportContent content{};
  content.manifest.producer_platform = request.producer_platform;
  content.manifest.producer_app = request.producer_app;
  content.manifest.created_at_utc = CurrentUtcTimestampRfc3339();
  content.manifest.source_root_name = kSourceRootName;
  content.entries =
      BuildConfigEntries(kConfigUserRoot, content.manifest.config_files);
  content.manifest.payload_files.reserve(kPayloadFiles.size());
  for (const auto& payload_file : kPayloadFiles) {
    content.manifest.payload_files.push_back(
        payload_file.relative_package_path);
  }

  content.entries.insert(content.entries.begin(),
                         BuildManifestEntry(content.manifest));
  for (const auto& payload_file : kPayloadFiles) {
    content.entries.push_back(BuildPayloadEntry(payload_file));
  }

  return {.ok = true, .content = std::move(content), .error_message = {}};
}

auto TracerExchangeService::EncodeExportContent(
    const app_dto::TracerExchangeExportContent& content)
    -> app_dto::TracerExchangeContentEncodingResult {
  std::vector<TracerExchangePackageEntry> package_entries;
  package_entries.reserve(content.entries.size());
  for (const auto& content_entry : content.entries) {
    package_entries.push_back(ToPackageEntry(content_entry));
  }

  return {
      .ok = true,
      .content = {.package_bytes = EncodePackageBytes(package_entries)},
      .error_message = {},
  };
}

auto TracerExchangeService::RunExport(
    const app_dto::TracerExchangeExportRequest& request)
    -> app_dto::TracerExchangeExportResult {
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
    if (UsesExistingProtection(request.protection)) {
      throw std::invalid_argument("Passphrase must not be empty.");
    }
  }
  EnsureSupportedProtection(request.protection);
  if (request.producer_platform.empty() || request.producer_app.empty()) {
    throw std::invalid_argument(
        "producer_platform/producer_app must not be empty.");
  }

  const fs::path kInputPath = request.input_text_root_path.empty()
                                  ? fs::path{}
                                  : fs::absolute(request.input_text_root_path);
  const std::string kSourceRootName =
      !request.logical_source_root_name.empty()
          ? request.logical_source_root_name
          : (!kInputPath.empty() && !kInputPath.filename().empty()
                 ? kInputPath.filename().string()
                 : std::string("data"));
  const fs::path kConfigUserRoot =
      fs::absolute(request.active_converter_main_config_path).parent_path();

  const auto kContentResult = BuildExportContent({
      .config_user_root_path = kConfigUserRoot,
      .active_converter_main_config_path =
          request.active_converter_main_config_path,
      .input_text_root_path = request.input_text_root_path,
      .input_text_payloads = request.input_text_payloads,
      .date_check_mode = request.date_check_mode,
      .logical_source_root_name = kSourceRootName,
      .producer_platform = request.producer_platform,
      .producer_app = request.producer_app,
  });

  const auto kEncodingResult = EncodeExportContent(kContentResult.content);
  const std::vector<std::uint8_t>& k_package_bytes =
      kEncodingResult.content.package_bytes;

  const fs::path kResolvedOutput =
      UsesExistingProtection(request.protection)
          ? (kHasOutputPath ? ResolveEncryptOutputPath({
                                  .input_path = kInputPath.empty()
                                                    ? fs::path(kSourceRootName)
                                                    : kInputPath,
                                  .output_arg = fs::absolute(
                                      request.requested_output_path),
                              })
                            : fs::path("android_export_sink") /
                                  (request.output_display_name.empty()
                                       ? (kSourceRootName + ".zip")
                                       : request.output_display_name))
          : (kHasOutputPath
                 ? ResolveUnprotectedOutputPath(
                       kInputPath, kSourceRootName,
                       fs::absolute(request.requested_output_path))
                 : fs::path("android_export_sink") /
                       ResolveUnprotectedOutputName(
                           kSourceRootName, request.output_display_name));
  if (kHasOutputPath) {
    EnsureParentDirectory(kResolvedOutput);
  }
  ProtectAndWriteEncodedPackage(k_package_bytes, request, kResolvedOutput);
  return {
      .ok = true,
      .resolved_output_tracer_path = kResolvedOutput,
      .source_root_name = kContentResult.content.manifest.source_root_name,
      .payload_file_count = static_cast<std::uint64_t>(
          kContentResult.content.manifest.payload_files.size()),
      .converter_file_count = static_cast<std::uint64_t>(
          kContentResult.content.manifest.config_files.size()),
      .manifest_included = true,
      .error_message = "",
  };
}

}  // namespace tracer_core::infrastructure::crypto::tracer_exchange_internal
