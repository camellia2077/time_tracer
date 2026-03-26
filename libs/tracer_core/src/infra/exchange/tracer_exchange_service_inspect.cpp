#include "infra/exchange/tracer_exchange_service_internal.hpp"

#include <cstdint>
#include <string>
#include <string_view>

import tracer.core.infrastructure.exchange;

namespace tracer_core::infrastructure::crypto::tracer_exchange_internal {

namespace {

using exchange_pkg::DecodePackageBytes;

auto FindEntrySummary(const exchange_pkg::DecodedTracerExchangePackage& package,
                      std::string_view path)
    -> app_dto::TracerExchangeInspectEntrySummary {
  app_dto::TracerExchangeInspectEntrySummary summary{};
  summary.relative_path = std::string(path);
  for (const auto& entry : package.entries) {
    if (entry.relative_path == path) {
      summary.present = true;
      summary.size_bytes = static_cast<std::uint64_t>(entry.data.size());
      break;
    }
  }
  return summary;
}

auto BuildInspectResult(
    const fs::path& input_path, const file_crypto::TracerFileMetadata& metadata,
    const exchange_pkg::DecodedTracerExchangePackage& package)
    -> app_dto::TracerExchangeInspectResult {
  app_dto::TracerExchangeInspectResult result{};
  result.ok = true;
  result.input_tracer_path = input_path;
  result.outer_metadata = {
      .version = metadata.version,
      .kdf_id = metadata.kdf_id,
      .cipher_id = metadata.cipher_id,
      .compression_id = metadata.compression_id,
      .compression_level = metadata.compression_level,
      .ops_limit = metadata.ops_limit,
      .mem_limit_kib = metadata.mem_limit_kib,
      .plaintext_size = metadata.plaintext_size,
      .ciphertext_size = metadata.ciphertext_size,
  };
  result.package_type = package.manifest.package_type;
  result.package_version = package.manifest.package_version;
  result.producer_platform = package.manifest.producer_platform;
  result.producer_app = package.manifest.producer_app;
  result.created_at_utc = package.manifest.created_at_utc;
  result.source_root_name = package.manifest.source_root_name;
  result.payload_file_count =
      static_cast<std::uint64_t>(package.manifest.payload_files.size());
  result.payload_entries.reserve(package.manifest.payload_files.size());
  for (const auto& payload_path : package.manifest.payload_files) {
    result.payload_entries.push_back(FindEntrySummary(package, payload_path));
  }
  result.converter_entries = {
      FindEntrySummary(package, exchange_pkg::kConverterMainPath),
      FindEntrySummary(package, exchange_pkg::kAliasMappingPath),
      FindEntrySummary(package, exchange_pkg::kDurationRulesPath),
  };
  return result;
}

}  // namespace

auto TracerExchangeService::RunInspect(
    const app_dto::TracerExchangeInspectRequest& request)
    -> app_dto::TracerExchangeInspectResult {
  if (request.input_tracer_path.empty()) {
    throw std::invalid_argument("input_path is required.");
  }
  if (request.passphrase.empty()) {
    throw std::invalid_argument("Passphrase must not be empty.");
  }

  const fs::path kInputPath = fs::absolute(request.input_tracer_path);
  if (!fs::exists(kInputPath) || !fs::is_regular_file(kInputPath)) {
    throw std::invalid_argument(
        "Inspect input path must be an existing file: " + kInputPath.string());
  }
  if (!HasExtensionCaseInsensitive(kInputPath, ".tracer")) {
    throw std::invalid_argument("Inspect input file must be .tracer: " +
                                kInputPath.string());
  }

  file_crypto::TracerFileMetadata metadata{};
  EnsureCryptoResultOk(file_crypto::InspectEncryptedFile(kInputPath, &metadata),
                       "Inspect", kInputPath);

  const std::string kStem = kInputPath.stem().empty()
                                ? kInputPath.filename().string()
                                : kInputPath.stem().string();
  const fs::path kStagingDir =
      BuildScopedStagingDir(kInputPath.parent_path(), "inspect", kStem);
  const fs::path kPackagePath = kStagingDir / "exchange.ttpkg";

  std::error_code io_error;
  fs::create_directories(kStagingDir, io_error);
  if (io_error) {
    throw std::runtime_error("Failed to create tracer exchange staging dir: " +
                             kStagingDir.string() + " | " + io_error.message());
  }

  try {
    EnsureCryptoResultOk(
        file_crypto::DecryptFile(
            kInputPath, kPackagePath, request.passphrase,
            BuildCryptoOptions(
                app_dto::TracerExchangeSecurityLevel::kInteractive,
                request.progress_observer)),
        "Inspect", kInputPath);
    const exchange_pkg::DecodedTracerExchangePackage kPackage =
        DecodePackageBytes(ReadFileBytes(kPackagePath));
    RemoveDirectoryBestEffort(kStagingDir);
    return BuildInspectResult(kInputPath, metadata, kPackage);
  } catch (...) {
    RemoveDirectoryBestEffort(kStagingDir);
    throw;
  }
}

}  // namespace tracer_core::infrastructure::crypto::tracer_exchange_internal
