#include "infra/exchange/tracer_exchange_service_internal.hpp"

#include <stdexcept>

import tracer.core.infrastructure.exchange;

namespace tracer_core::infrastructure::crypto::tracer_exchange_internal {

namespace {

using exchange_pkg::DecodePackageBytes;

auto EnsureUnpackOutputRootReady(const fs::path& output_root) -> void {
  if (output_root.empty()) {
    throw std::invalid_argument("requested_output_root_path must not be empty.");
  }
  if (fs::exists(output_root)) {
    if (!fs::is_directory(output_root)) {
      throw std::invalid_argument(
          "Unpack output path must be a directory: " + output_root.string());
    }
    if (fs::directory_iterator(output_root) != fs::directory_iterator()) {
      throw std::invalid_argument(
          "Unpack output directory must be empty: " + output_root.string());
    }
    return;
  }

  std::error_code error;
  fs::create_directories(output_root, error);
  if (error) {
    throw std::runtime_error("Failed to create unpack output directory: " +
                             output_root.string() + " | " + error.message());
  }
}

}  // namespace

auto TracerExchangeService::RunUnpack(
    const app_dto::TracerExchangeUnpackRequest& request)
    -> app_dto::TracerExchangeUnpackResult {
  if (request.input_tracer_path.empty()) {
    throw std::invalid_argument("input_path is required.");
  }
  if (request.passphrase.empty()) {
    throw std::invalid_argument("Passphrase must not be empty.");
  }
  if (request.requested_output_root_path.empty()) {
    throw std::invalid_argument("requested_output_root_path must not be empty.");
  }

  const fs::path kInputPath = fs::absolute(request.input_tracer_path);
  const fs::path kOutputRoot = fs::absolute(request.requested_output_root_path);
  if (!fs::exists(kInputPath) || !fs::is_regular_file(kInputPath)) {
    throw std::invalid_argument(
        "Unpack input path must be an existing file: " + kInputPath.string());
  }
  if (!HasExtensionCaseInsensitive(kInputPath, ".tracer")) {
    throw std::invalid_argument("Unpack input file must be .tracer: " +
                                kInputPath.string());
  }
  EnsureUnpackOutputRootReady(kOutputRoot);

  const file_crypto::FileCryptoPathContext kPathContext{
      .input_root_path = kInputPath.parent_path(),
      .output_root_path = kOutputRoot,
      .current_input_path = kInputPath,
      .current_output_path = kOutputRoot / "exchange.ttpkg",
  };
  auto [decrypt_result, package_bytes] = file_crypto::DecryptFileToBytes(
      kInputPath, request.passphrase, kPathContext,
      BuildCryptoOptions(app_dto::TracerExchangeSecurityLevel::kInteractive,
                         request.progress_observer));
  EnsureCryptoResultOk(decrypt_result, "Unpack", kInputPath);

  const exchange_pkg::DecodedTracerExchangePackage kPackage =
      DecodePackageBytes(package_bytes);
  WriteDecodedPackageToRoot(kPackage, kOutputRoot);

  return {
      .ok = true,
      .resolved_output_root_path = kOutputRoot,
      .source_root_name = kPackage.manifest.source_root_name,
      .payload_file_count =
          static_cast<std::uint64_t>(kPackage.manifest.payload_files.size()),
      .converter_file_count =
          static_cast<std::uint64_t>(
              3U + kPackage.manifest.converter_alias_mapping_files.size()),
      .manifest_included = true,
      .error_message = "",
  };
}

}  // namespace tracer_core::infrastructure::crypto::tracer_exchange_internal
