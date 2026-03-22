#include "infra/crypto/tracer_exchange_service_internal.hpp"

#include <cstdint>
#include <string>
#include <string_view>

import tracer.core.infrastructure.crypto.exchange;

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

auto BuildInspectResult(const fs::path& input_path,
                        const file_crypto::TracerFileMetadata& metadata,
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

  const fs::path input_path = fs::absolute(request.input_tracer_path);
  if (!fs::exists(input_path) || !fs::is_regular_file(input_path)) {
    throw std::invalid_argument("Inspect input path must be an existing file: " +
                                input_path.string());
  }
  if (!HasExtensionCaseInsensitive(input_path, ".tracer")) {
    throw std::invalid_argument("Inspect input file must be .tracer: " +
                                input_path.string());
  }

  file_crypto::TracerFileMetadata metadata{};
  EnsureCryptoResultOk(file_crypto::InspectEncryptedFile(input_path, &metadata),
                       "Inspect", input_path);

  const std::string stem =
      input_path.stem().empty() ? input_path.filename().string()
                                : input_path.stem().string();
  const fs::path staging_dir =
      BuildScopedStagingDir(input_path.parent_path(), "inspect", stem);
  const fs::path package_path = staging_dir / "exchange.ttpkg";

  std::error_code io_error;
  fs::create_directories(staging_dir, io_error);
  if (io_error) {
    throw std::runtime_error("Failed to create tracer exchange staging dir: " +
                             staging_dir.string() + " | " + io_error.message());
  }

  try {
    EnsureCryptoResultOk(
        file_crypto::DecryptFile(
            input_path, package_path, request.passphrase,
            BuildCryptoOptions(app_dto::TracerExchangeSecurityLevel::kInteractive,
                               request.progress_observer)),
        "Inspect", input_path);
    const exchange_pkg::DecodedTracerExchangePackage package =
        DecodePackageBytes(ReadFileBytes(package_path));
    RemoveDirectoryBestEffort(staging_dir);
    return BuildInspectResult(input_path, metadata, package);
  } catch (...) {
    RemoveDirectoryBestEffort(staging_dir);
    throw;
  }
}

}  // namespace tracer_core::infrastructure::crypto::tracer_exchange_internal
