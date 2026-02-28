// infrastructure/crypto/file_crypto_service.cpp
#include "infrastructure/crypto/file_crypto_service.hpp"

#include "infrastructure/crypto/internal/file_crypto_backend_engine.hpp"
#include "infrastructure/crypto/internal/file_crypto_common.hpp"
#include "infrastructure/crypto/internal/file_crypto_directory_orchestrator.hpp"
#include "infrastructure/crypto/internal/file_crypto_directory_plan.hpp"
#include "infrastructure/crypto/internal/file_crypto_format_compat.hpp"
#include "infrastructure/crypto/internal/file_crypto_io.hpp"
#include "infrastructure/crypto/internal/file_crypto_progress_control.hpp"

#ifdef _WIN32
#ifdef EncryptFile
#undef EncryptFile
#endif
#ifdef DecryptFile
#undef DecryptFile
#endif
#endif

namespace tracer_core::infrastructure::crypto {

namespace file_crypto_internal = tracer_core::infrastructure::crypto::internal;

auto ToErrorCode(FileCryptoError error) -> std::string_view {
  switch (error) {
    case FileCryptoError::kOk:
      return "ok";
    case FileCryptoError::kInvalidArgument:
      return "crypto.invalid_argument";
    case FileCryptoError::kInputReadFailed:
      return "crypto.input_read_failed";
    case FileCryptoError::kOutputWriteFailed:
      return "crypto.output_write_failed";
    case FileCryptoError::kUnsupportedFormat:
      return "crypto.unsupported_format";
    case FileCryptoError::kDecryptFailed:
      return "crypto.decrypt_failed";
    case FileCryptoError::kCompressionFailed:
      return "crypto.compress_failed";
    case FileCryptoError::kDecompressionFailed:
      return "crypto.decompress_failed";
    case FileCryptoError::kCompressionMetadataMismatch:
      return "crypto.compression_metadata_mismatch";
    case FileCryptoError::kCryptoBackendUnavailable:
      return "crypto.backend_unavailable";
    case FileCryptoError::kCryptoInitializationFailed:
      return "crypto.init_failed";
    case FileCryptoError::kCryptoOperationFailed:
      return "crypto.operation_failed";
    case FileCryptoError::kCancelled:
      return "crypto.cancelled";
  }
  return "crypto.unknown";
}

auto EncryptFile(const std::filesystem::path& input_txt_path,
                 const std::filesystem::path& output_tracer_path,
                 std::string_view passphrase) -> FileCryptoResult {
  const FileCryptoOptions kOptions{};
  return EncryptFile(input_txt_path, output_tracer_path, passphrase, kOptions);
}

auto EncryptFile(const std::filesystem::path& input_txt_path,
                 const std::filesystem::path& output_tracer_path,
                 std::string_view passphrase, const FileCryptoOptions& options)
    -> FileCryptoResult {
  if (input_txt_path.empty() || output_tracer_path.empty()) {
    return file_crypto_internal::MakeError(
        FileCryptoError::kInvalidArgument,
        "Input and output paths are required.");
  }
  if (passphrase.empty()) {
    return file_crypto_internal::MakeError(FileCryptoError::kInvalidArgument,
                                           "Passphrase must not be empty.");
  }

  auto [entry_result, entry] = file_crypto_internal::BuildSingleFilePlanEntry(
      input_txt_path, output_tracer_path);
  if (!entry_result.ok()) {
    return entry_result;
  }

  file_crypto_internal::ProgressReporter reporter(FileCryptoOperation::kEncrypt,
                                                  &options);
  if (const auto kScanResult = reporter.BeginScan(
          input_txt_path.parent_path(), output_tracer_path.parent_path());
      !kScanResult.ok()) {
    return kScanResult;
  }
  if (const auto kTotalsResult =
          reporter.SetAggregateTotals(1, entry.input_size_bytes, 1, true);
      !kTotalsResult.ok()) {
    return kTotalsResult;
  }

  file_crypto_internal::ProgressFileDescriptor descriptor{};
  descriptor.input_path = entry.input_path;
  descriptor.output_path = entry.output_path;
  descriptor.group_label = entry.group_label;
  descriptor.group_index = entry.group_index;
  descriptor.group_file_index = entry.group_file_index;
  descriptor.group_file_count = entry.group_file_count;
  descriptor.file_index = entry.file_index;
  descriptor.total_files = entry.total_files;
  descriptor.input_size_bytes = entry.input_size_bytes;
  if (const auto kSelectResult = reporter.SetCurrentFile(descriptor, 0);
      !kSelectResult.ok()) {
    return kSelectResult;
  }

  const auto kResult = file_crypto_internal::EncryptFileInternal(
      input_txt_path, output_tracer_path, passphrase, options.security_level,
      &reporter);
  if (!kResult.ok()) {
    if (file_crypto_internal::IsCancelledError(kResult)) {
      (void)reporter.MarkCancelled();
    } else {
      (void)reporter.MarkFailed();
    }
    return kResult;
  }
  if (const auto kCompleteResult = reporter.MarkCompleted();
      !kCompleteResult.ok()) {
    return kCompleteResult;
  }
  return kResult;
}

auto DecryptFile(const std::filesystem::path& input_tracer_path,
                 const std::filesystem::path& output_txt_path,
                 std::string_view passphrase) -> FileCryptoResult {
  const FileCryptoOptions kOptions{};
  return DecryptFile(input_tracer_path, output_txt_path, passphrase, kOptions);
}

auto DecryptFile(const std::filesystem::path& input_tracer_path,
                 const std::filesystem::path& output_txt_path,
                 std::string_view passphrase, const FileCryptoOptions& options)
    -> FileCryptoResult {
  if (input_tracer_path.empty() || output_txt_path.empty()) {
    return file_crypto_internal::MakeError(
        FileCryptoError::kInvalidArgument,
        "Input and output paths are required.");
  }
  if (passphrase.empty()) {
    return file_crypto_internal::MakeError(FileCryptoError::kInvalidArgument,
                                           "Passphrase must not be empty.");
  }

  auto [entry_result, entry] = file_crypto_internal::BuildSingleFilePlanEntry(
      input_tracer_path, output_txt_path);
  if (!entry_result.ok()) {
    return entry_result;
  }

  file_crypto_internal::ProgressReporter reporter(FileCryptoOperation::kDecrypt,
                                                  &options);
  if (const auto kScanResult = reporter.BeginScan(
          input_tracer_path.parent_path(), output_txt_path.parent_path());
      !kScanResult.ok()) {
    return kScanResult;
  }
  if (const auto kTotalsResult =
          reporter.SetAggregateTotals(1, entry.input_size_bytes, 1, true);
      !kTotalsResult.ok()) {
    return kTotalsResult;
  }

  file_crypto_internal::ProgressFileDescriptor descriptor{};
  descriptor.input_path = entry.input_path;
  descriptor.output_path = entry.output_path;
  descriptor.group_label = entry.group_label;
  descriptor.group_index = entry.group_index;
  descriptor.group_file_index = entry.group_file_index;
  descriptor.group_file_count = entry.group_file_count;
  descriptor.file_index = entry.file_index;
  descriptor.total_files = entry.total_files;
  descriptor.input_size_bytes = entry.input_size_bytes;
  if (const auto kSelectResult = reporter.SetCurrentFile(descriptor, 0);
      !kSelectResult.ok()) {
    return kSelectResult;
  }

  const auto kResult = file_crypto_internal::DecryptFileInternal(
      input_tracer_path, output_txt_path, passphrase, &reporter);
  if (!kResult.ok()) {
    if (file_crypto_internal::IsCancelledError(kResult)) {
      (void)reporter.MarkCancelled();
    } else {
      (void)reporter.MarkFailed();
    }
    return kResult;
  }
  if (const auto kCompleteResult = reporter.MarkCompleted();
      !kCompleteResult.ok()) {
    return kCompleteResult;
  }
  return kResult;
}

auto EncryptDirectory(const std::filesystem::path& input_root_path,
                      const std::filesystem::path& output_root_path,
                      std::string_view passphrase,
                      const FileCryptoOptions& options)
    -> FileCryptoBatchResult {
  return file_crypto_internal::RunDirectoryCrypto(
      FileCryptoOperation::kEncrypt, input_root_path, output_root_path,
      passphrase, ".txt", ".tracer", options);
}

auto DecryptDirectory(const std::filesystem::path& input_root_path,
                      const std::filesystem::path& output_root_path,
                      std::string_view passphrase,
                      const FileCryptoOptions& options)
    -> FileCryptoBatchResult {
  return file_crypto_internal::RunDirectoryCrypto(
      FileCryptoOperation::kDecrypt, input_root_path, output_root_path,
      passphrase, ".tracer", ".txt", options);
}

auto InspectEncryptedFile(const std::filesystem::path& input_tracer_path,
                          TracerFileMetadata* metadata_out)
    -> FileCryptoResult {
  if (input_tracer_path.empty()) {
    return file_crypto_internal::MakeError(FileCryptoError::kInvalidArgument,
                                           "Input path is required.");
  }

  auto [read_result, encrypted_bytes] =
      file_crypto_internal::ReadAllBytes(input_tracer_path, nullptr);
  if (!read_result.ok()) {
    return read_result;
  }

  file_crypto_internal::TracerFileHeader header{};
  if (const auto kParseResult =
          file_crypto_internal::ParseHeader(encrypted_bytes, header);
      !kParseResult.ok()) {
    return kParseResult;
  }

  if (metadata_out != nullptr) {
    metadata_out->version = header.version;
    metadata_out->kdf_id = header.kdf_id;
    metadata_out->cipher_id = header.cipher_id;
    metadata_out->compression_id = header.compression_id;
    metadata_out->compression_level = header.compression_level;
    metadata_out->ops_limit = header.ops_limit;
    metadata_out->mem_limit_kib = header.mem_limit_kib;
    metadata_out->plaintext_size = header.plaintext_size;
    metadata_out->ciphertext_size = header.ciphertext_size;
  }
  return {};
}

}  // namespace tracer_core::infrastructure::crypto
