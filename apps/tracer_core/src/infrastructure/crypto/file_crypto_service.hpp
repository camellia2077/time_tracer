// infrastructure/crypto/file_crypto_service.hpp
#ifndef INFRASTRUCTURE_CRYPTO_FILE_CRYPTO_SERVICE_HPP_
#define INFRASTRUCTURE_CRYPTO_FILE_CRYPTO_SERVICE_HPP_

#include <atomic>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

#if defined(_WIN32)
#if defined(EncryptFile)
#undef EncryptFile
#endif
#if defined(DecryptFile)
#undef DecryptFile
#endif
#endif

namespace tracer_core::infrastructure::crypto {

enum class FileCryptoError {
  kOk = 0,
  kInvalidArgument,
  kInputReadFailed,
  kOutputWriteFailed,
  kUnsupportedFormat,
  kDecryptFailed,
  kCompressionFailed,
  kDecompressionFailed,
  kCompressionMetadataMismatch,
  kCryptoBackendUnavailable,
  kCryptoInitializationFailed,
  kCryptoOperationFailed,
  kCancelled,
};

struct FileCryptoResult {
  FileCryptoError error = FileCryptoError::kOk;
  std::string error_code;
  std::string error_message;

  [[nodiscard]] auto ok() const -> bool {
    return error == FileCryptoError::kOk;
  }
};

enum class FileCryptoOperation {
  kEncrypt = 0,
  kDecrypt,
};

enum class FileCryptoPhase {
  kScan = 0,
  kReadInput,
  kCompress,
  kDeriveKey,
  kEncrypt,
  kDecrypt,
  kDecompress,
  kWriteOutput,
  kCompleted,
  kCancelled,
  kFailed,
};

enum class FileCryptoControl {
  kContinue = 0,
  kCancel,
};

enum class FileCryptoSecurityLevel {
  kInteractive = 0,
  kModerate,
  kHigh,
};

struct FileCryptoProgressSnapshot {
  FileCryptoOperation operation = FileCryptoOperation::kEncrypt;
  FileCryptoPhase phase = FileCryptoPhase::kScan;
  std::filesystem::path input_root_path;
  std::filesystem::path output_root_path;
  std::filesystem::path current_input_path;
  std::filesystem::path current_output_path;
  std::string current_group_label;
  std::size_t group_index = 0;
  std::size_t group_count = 0;
  std::size_t file_index_in_group = 0;
  std::size_t file_count_in_group = 0;
  std::size_t current_file_index = 0;
  std::size_t total_files = 0;
  std::uint64_t current_file_done_bytes = 0;
  std::uint64_t current_file_total_bytes = 0;
  std::uint64_t overall_done_bytes = 0;
  std::uint64_t overall_total_bytes = 0;
  std::uint64_t speed_bytes_per_sec = 0;
  std::uint64_t remaining_bytes = 0;
  std::uint64_t eta_seconds = 0;
};

using FileCryptoProgressCallback =
    std::function<FileCryptoControl(const FileCryptoProgressSnapshot&)>;

struct FileCryptoCancelToken {
  std::atomic<bool> cancelled{false};

  auto RequestCancel() -> void {
    cancelled.store(true, std::memory_order_relaxed);
  }

  [[nodiscard]] auto IsCancelled() const -> bool {
    return cancelled.load(std::memory_order_relaxed);
  }
};

struct FileCryptoOptions {
  FileCryptoProgressCallback progress_callback = nullptr;
  const FileCryptoCancelToken* cancel_token = nullptr;
  FileCryptoSecurityLevel security_level =
      FileCryptoSecurityLevel::kInteractive;
  std::chrono::milliseconds progress_min_interval{100};
  std::uint64_t progress_min_bytes_delta = 64U * 1024U;
  bool continue_on_error = false;
};

struct FileCryptoBatchFileError {
  std::filesystem::path input_path;
  std::filesystem::path output_path;
  std::string error_code;
  std::string error_message;
};

struct FileCryptoBatchResult {
  FileCryptoResult status{};
  std::size_t total_files = 0;
  std::size_t succeeded_files = 0;
  std::size_t failed_files = 0;
  bool cancelled = false;
  std::vector<FileCryptoBatchFileError> file_errors;

  [[nodiscard]] auto ok() const -> bool {
    return status.ok() && failed_files == 0 && !cancelled;
  }
};

struct TracerFileMetadata {
  std::uint8_t version = 0;
  std::uint8_t kdf_id = 0;
  std::uint8_t cipher_id = 0;
  std::uint8_t compression_id = 0;
  std::uint8_t compression_level = 0;
  std::uint32_t ops_limit = 0;
  std::uint32_t mem_limit_kib = 0;
  std::uint64_t plaintext_size = 0;
  std::uint64_t ciphertext_size = 0;
};

auto ToErrorCode(FileCryptoError error) -> std::string_view;

auto EncryptFile(const std::filesystem::path& input_txt_path,
                 const std::filesystem::path& output_tracer_path,
                 std::string_view passphrase) -> FileCryptoResult;

auto EncryptFile(const std::filesystem::path& input_txt_path,
                 const std::filesystem::path& output_tracer_path,
                 std::string_view passphrase, const FileCryptoOptions& options)
    -> FileCryptoResult;

auto DecryptFile(const std::filesystem::path& input_tracer_path,
                 const std::filesystem::path& output_txt_path,
                 std::string_view passphrase) -> FileCryptoResult;

auto DecryptFile(const std::filesystem::path& input_tracer_path,
                 const std::filesystem::path& output_txt_path,
                 std::string_view passphrase, const FileCryptoOptions& options)
    -> FileCryptoResult;

auto EncryptDirectory(const std::filesystem::path& input_root_path,
                      const std::filesystem::path& output_root_path,
                      std::string_view passphrase,
                      const FileCryptoOptions& options = FileCryptoOptions{})
    -> FileCryptoBatchResult;

auto DecryptDirectory(const std::filesystem::path& input_root_path,
                      const std::filesystem::path& output_root_path,
                      std::string_view passphrase,
                      const FileCryptoOptions& options = FileCryptoOptions{})
    -> FileCryptoBatchResult;

auto InspectEncryptedFile(const std::filesystem::path& input_tracer_path,
                          TracerFileMetadata* metadata_out) -> FileCryptoResult;

}  // namespace tracer_core::infrastructure::crypto

#endif  // INFRASTRUCTURE_CRYPTO_FILE_CRYPTO_SERVICE_HPP_
