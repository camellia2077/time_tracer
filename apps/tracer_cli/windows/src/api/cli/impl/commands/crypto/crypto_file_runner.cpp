// api/cli/impl/commands/crypto/crypto_file_runner.cpp
#include "api/cli/impl/commands/crypto/crypto_file_runner.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "api/cli/impl/presentation/progress/crypto_progress_renderer.hpp"
#include "application/dto/core_requests.hpp"
#include "infrastructure/crypto/file_crypto_service.hpp"
#include "shared/types/exceptions.hpp"

namespace tracer_core::cli::impl::commands::crypto {
namespace {

namespace fs = std::filesystem;
namespace file_crypto = tracer_core::infrastructure::crypto;
namespace progress_presentation =
    tracer_core::cli::impl::presentation::progress;

[[nodiscard]] auto ToLowerAscii(std::string value) -> std::string {
  std::ranges::transform(value, value.begin(), [](unsigned char ch) -> char {
    return static_cast<char>(std::tolower(ch));
  });
  return value;
}

[[nodiscard]] auto ToCoreSecurityLevel(CryptoSecurityLevel security_level)
    -> file_crypto::FileCryptoSecurityLevel {
  switch (security_level) {
  case CryptoSecurityLevel::kModerate:
    return file_crypto::FileCryptoSecurityLevel::kModerate;
  case CryptoSecurityLevel::kHigh:
    return file_crypto::FileCryptoSecurityLevel::kHigh;
  case CryptoSecurityLevel::kInteractive:
  default:
    return file_crypto::FileCryptoSecurityLevel::kInteractive;
  }
}

[[nodiscard]] auto HasExtensionCaseInsensitive(const fs::path &path,
                                               std::string_view ext_lower)
    -> bool {
  return ToLowerAscii(path.extension().string()) == ext_lower;
}

void EnsureOperationSuccess(
    const tracer_core::core::dto::OperationAck &response,
    std::string_view fallback_message) {
  if (response.ok) {
    return;
  }
  if (!response.error_message.empty()) {
    throw tracer_core::common::LogicError(response.error_message);
  }
  throw tracer_core::common::LogicError(std::string(fallback_message));
}

void ValidateInputTxtForEncryption(ITracerCoreApi &core_api,
                                   DateCheckMode date_check_mode,
                                   const fs::path &input_path) {
  const std::string input = input_path.string();
  const auto structure_ack =
      core_api.RunValidateStructure({.input_path = input});
  EnsureOperationSuccess(structure_ack,
                         "TXT structure validation failed before encryption.");

  const auto logic_ack = core_api.RunValidateLogic(
      {.input_path = input, .date_check_mode = date_check_mode});
  EnsureOperationSuccess(logic_ack,
                         "TXT logic validation failed before encryption.");
}

[[nodiscard]] auto ResolveFileOutputPath(const fs::path &input_file,
                                         const fs::path &output_arg,
                                         std::string_view ext_lower)
    -> fs::path {
  fs::path output_path = output_arg;
  if (fs::exists(output_path) && fs::is_directory(output_path)) {
    output_path /= input_file.filename();
  }
  output_path.replace_extension(ext_lower);
  return output_path;
}

[[nodiscard]] auto CollectFilesByExtension(const fs::path &root,
                                           std::string_view ext_lower)
    -> std::vector<fs::path> {
  std::vector<fs::path> files;
  if (!fs::exists(root)) {
    throw std::runtime_error("Input path does not exist: " + root.string());
  }
  if (!fs::is_directory(root)) {
    throw std::runtime_error("Expected directory path: " + root.string());
  }

  for (const auto &entry : fs::recursive_directory_iterator(root)) {
    if (!entry.is_regular_file()) {
      continue;
    }
    if (HasExtensionCaseInsensitive(entry.path(), ext_lower)) {
      files.push_back(entry.path());
    }
  }
  return files;
}

void EnsureCryptoResultOk(const file_crypto::FileCryptoResult &result,
                          std::string_view action, const fs::path &input_path) {
  if (result.ok()) {
    return;
  }
  throw tracer_core::common::LogicError(
      std::string(action) + " failed for " + input_path.string() + ": " +
      result.error_code + " | " + result.error_message);
}

[[nodiscard]] auto BuildProgressOptions(
    progress_presentation::CryptoProgressRenderer &progress_renderer,
    CryptoSecurityLevel security_level = CryptoSecurityLevel::kInteractive)
    -> file_crypto::FileCryptoOptions {
  file_crypto::FileCryptoOptions options{};
  options.security_level = ToCoreSecurityLevel(security_level);
  options.progress_callback = progress_renderer.BuildCallback();
  return options;
}

[[nodiscard]] auto
BuildBatchFailureMessage(std::string_view action,
                         const file_crypto::FileCryptoBatchResult &batch)
    -> std::string {
  std::ostringstream message;
  message << action << " batch failed";
  if (!batch.status.error_code.empty() || !batch.status.error_message.empty()) {
    message << ": " << batch.status.error_code << " | "
            << batch.status.error_message;
  }
  if (batch.cancelled ||
      batch.status.error == file_crypto::FileCryptoError::kCancelled) {
    message << " (cancelled)";
  }
  if (!batch.file_errors.empty()) {
    const auto &first_error = batch.file_errors.front();
    message << " | file_errors=" << batch.file_errors.size()
            << " | first=" << first_error.input_path.string() << " => "
            << first_error.error_code << " | " << first_error.error_message;
  }
  return message.str();
}

void PrintMetadata(const fs::path &input_file) {
  file_crypto::TracerFileMetadata metadata{};
  const auto result = file_crypto::InspectEncryptedFile(input_file, &metadata);
  EnsureCryptoResultOk(result, "Inspect", input_file);
  std::cout << "File: " << input_file.string() << '\n';
  std::cout << "  version: " << static_cast<int>(metadata.version) << '\n';
  std::cout << "  kdf_id: " << static_cast<int>(metadata.kdf_id) << '\n';
  std::cout << "  cipher_id: " << static_cast<int>(metadata.cipher_id) << '\n';
  std::cout << "  compression_id: " << static_cast<int>(metadata.compression_id)
            << '\n';
  std::cout << "  compression_level: "
            << static_cast<int>(metadata.compression_level) << '\n';
  std::cout << "  ops_limit: " << metadata.ops_limit << '\n';
  std::cout << "  mem_limit_kib: " << metadata.mem_limit_kib << '\n';
  std::cout << "  plaintext_size: " << metadata.plaintext_size << '\n';
  std::cout << "  ciphertext_size: " << metadata.ciphertext_size << '\n';
}

} // namespace

auto ParseCryptoAction(std::string_view action) -> std::optional<CryptoAction> {
  if (action == "encrypt") {
    return CryptoAction::kEncrypt;
  }
  if (action == "decrypt") {
    return CryptoAction::kDecrypt;
  }
  if (action == "inspect") {
    return CryptoAction::kInspect;
  }
  return std::nullopt;
}

auto ParseCryptoSecurityLevel(std::string_view value)
    -> std::optional<CryptoSecurityLevel> {
  const std::string normalized_value = ToLowerAscii(std::string(value));
  if (normalized_value == "interactive") {
    return CryptoSecurityLevel::kInteractive;
  }
  if (normalized_value == "moderate") {
    return CryptoSecurityLevel::kModerate;
  }
  if (normalized_value == "high") {
    return CryptoSecurityLevel::kHigh;
  }
  return std::nullopt;
}

auto RunCryptoEncrypt(ITracerCoreApi &core_api, DateCheckMode date_check_mode,
                      const fs::path &input_path, const fs::path &output_path,
                      std::string_view passphrase,
                      CryptoSecurityLevel security_level) -> void {
  progress_presentation::CryptoProgressRenderer progress_renderer{};
  const file_crypto::FileCryptoOptions crypto_options =
      BuildProgressOptions(progress_renderer, security_level);

  if (fs::is_regular_file(input_path)) {
    if (!HasExtensionCaseInsensitive(input_path, ".txt")) {
      throw std::runtime_error("Encrypt input file must be .txt: " +
                               input_path.string());
    }
    ValidateInputTxtForEncryption(core_api, date_check_mode, input_path);
    const fs::path resolved_output =
        ResolveFileOutputPath(input_path, output_path, ".tracer");
    EnsureCryptoResultOk(file_crypto::EncryptFile(input_path, resolved_output,
                                                  passphrase, crypto_options),
                         "Encrypt", input_path);
    std::cout << "Encrypted: " << input_path.string() << " -> "
              << resolved_output.string() << '\n';
    return;
  }

  if (!fs::is_directory(input_path)) {
    throw std::runtime_error("Encrypt input path must be file or directory.");
  }
  if (fs::exists(output_path) && fs::is_regular_file(output_path)) {
    throw std::runtime_error(
        "Encrypt output path must be a directory when input is directory: " +
        output_path.string());
  }
  // Keep existing contract: encryption validates source before crypto.
  ValidateInputTxtForEncryption(core_api, date_check_mode, input_path);

  const file_crypto::FileCryptoBatchResult batch =
      file_crypto::EncryptDirectory(input_path, output_path, passphrase,
                                    crypto_options);
  if (!batch.ok()) {
    std::cerr << "[crypto] encrypt | status: failed | reason: batch_error\n";
    throw tracer_core::common::LogicError(
        BuildBatchFailureMessage("Encrypt", batch));
  }
  std::cout << "Encrypted " << batch.succeeded_files
            << " txt files to: " << output_path.string() << '\n';
}

auto RunCryptoDecrypt(const fs::path &input_path, const fs::path &output_path,
                      std::string_view passphrase) -> void {
  progress_presentation::CryptoProgressRenderer progress_renderer{};
  const file_crypto::FileCryptoOptions crypto_options =
      BuildProgressOptions(progress_renderer);

  if (fs::is_regular_file(input_path)) {
    if (!HasExtensionCaseInsensitive(input_path, ".tracer")) {
      throw std::runtime_error("Decrypt input file must be .tracer: " +
                               input_path.string());
    }
    const fs::path resolved_output =
        ResolveFileOutputPath(input_path, output_path, ".txt");
    EnsureCryptoResultOk(file_crypto::DecryptFile(input_path, resolved_output,
                                                  passphrase, crypto_options),
                         "Decrypt", input_path);
    std::cout << "Decrypted: " << input_path.string() << " -> "
              << resolved_output.string() << '\n';
    return;
  }

  if (!fs::is_directory(input_path)) {
    throw std::runtime_error("Decrypt input path must be file or directory.");
  }
  if (fs::exists(output_path) && fs::is_regular_file(output_path)) {
    throw std::runtime_error(
        "Decrypt output path must be a directory when input is directory: " +
        output_path.string());
  }
  const file_crypto::FileCryptoBatchResult batch =
      file_crypto::DecryptDirectory(input_path, output_path, passphrase,
                                    crypto_options);
  if (!batch.ok()) {
    throw tracer_core::common::LogicError(
        BuildBatchFailureMessage("Decrypt", batch));
  }
  std::cout << "Decrypted " << batch.succeeded_files
            << " tracer files to: " << output_path.string() << '\n';
}

auto RunCryptoInspect(const fs::path &input_path) -> void {
  if (fs::is_regular_file(input_path)) {
    PrintMetadata(input_path);
    return;
  }
  if (!fs::is_directory(input_path)) {
    throw std::runtime_error("Inspect input path must be file or directory.");
  }
  const auto tracer_files = CollectFilesByExtension(input_path, ".tracer");
  if (tracer_files.empty()) {
    throw std::runtime_error("No .tracer files found under directory: " +
                             input_path.string());
  }
  for (const auto &file : tracer_files) {
    PrintMetadata(file);
  }
}

} // namespace tracer_core::cli::impl::commands::crypto
