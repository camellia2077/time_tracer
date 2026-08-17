#include "infra/exchange/tracer_exchange_service_internal.hpp"

#include <cstdint>
#include <string>
#include <string_view>

import tracer.core.infrastructure.exchange;

namespace tracer_core::infrastructure::crypto::tracer_exchange_internal {

namespace {

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
    const fs::path& input_path, std::uint64_t archive_size,
    const exchange_pkg::DecodedTracerExchangePackage& package)
    -> app_dto::TracerExchangeInspectResult {
  app_dto::TracerExchangeInspectResult result{};
  result.ok = true;
  result.input_tracer_path = input_path;
  result.outer_metadata.version = 3U;
  result.outer_metadata.kdf_id = 1U;          // PBKDF2-HMAC-SHA1 (ZIP AES)
  result.outer_metadata.cipher_id = 3U;       // AES-256-CTR + HMAC-SHA1
  result.outer_metadata.compression_id = 8U;  // ZIP deflate
  result.outer_metadata.ops_limit = 1000U;    // ZIP AES PBKDF2 rounds
  result.outer_metadata.plaintext_size = 0U;
  for (const auto& entry : package.entries) {
    result.outer_metadata.plaintext_size += entry.data.size();
  }
  result.outer_metadata.ciphertext_size = archive_size;
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
  result.config_entries.reserve(package.manifest.config_files.size());
  for (const auto& config_path : package.manifest.config_files) {
    result.config_entries.push_back(FindEntrySummary(package, config_path));
  }
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
  if (!HasExtensionCaseInsensitive(kInputPath, ".zip")) {
    throw std::invalid_argument("Inspect input file must be .zip: " +
                                kInputPath.string());
  }

  const auto kEncryptedZip = ReadFileBytes(kInputPath);
  const exchange_pkg::DecodedTracerExchangePackage kPackage =
      exchange_pkg::DecodeZipBytes(kEncryptedZip, request.passphrase);
  return BuildInspectResult(kInputPath, kEncryptedZip.size(), kPackage);
}

}  // namespace tracer_core::infrastructure::crypto::tracer_exchange_internal
