// infra/crypto/file_crypto_service.cpp
#include "infra/crypto/file_crypto_service.hpp"

#include "infra/crypto/internal/file_crypto_backend_engine.hpp"
#include "infra/crypto/internal/file_crypto_common.hpp"
#include "infra/crypto/internal/file_crypto_directory_orchestrator.hpp"
#include "infra/crypto/internal/file_crypto_directory_plan.hpp"
#include "infra/crypto/internal/file_crypto_format_compat.hpp"
#include "infra/crypto/internal/file_crypto_io.hpp"
#include "infra/crypto/internal/file_crypto_progress_control.hpp"

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

namespace {

auto ResolvePathOrFallback(const std::filesystem::path& candidate,
                           const std::filesystem::path& fallback)
    -> std::filesystem::path {
  return candidate.empty() ? fallback : candidate;
}

auto ResolveRootOrFallback(const std::filesystem::path& candidate,
                           const std::filesystem::path& fallback_path)
    -> std::filesystem::path {
  if (!candidate.empty()) {
    return candidate;
  }
  if (!fallback_path.empty()) {
    return fallback_path.parent_path();
  }
  return {};
}

auto ResolveGroupLabel(const std::filesystem::path& input_root_path,
                       const std::filesystem::path& current_input_path)
    -> std::string {
  if (input_root_path.empty() || current_input_path.empty()) {
    return "(root)";
  }
  std::error_code relative_error;
  const std::filesystem::path relative =
      std::filesystem::relative(current_input_path, input_root_path,
                                relative_error);
  if (relative_error) {
    return "(root)";
  }
  const std::filesystem::path parent = relative.parent_path();
  if (parent.empty() || parent == ".") {
    return "(root)";
  }
  const auto it = parent.begin();
  if (it == parent.end()) {
    return "(root)";
  }
  const std::string label = it->string();
  return label.empty() ? "(root)" : label;
}

auto BuildSingleFileDescriptor(const FileCryptoPathContext& path_context,
                               const std::filesystem::path& fallback_input_path,
                               const std::filesystem::path& fallback_output_path,
                               std::uint64_t input_size_bytes)
    -> file_crypto_internal::ProgressFileDescriptor {
  file_crypto_internal::ProgressFileDescriptor descriptor{};
  descriptor.input_path = ResolvePathOrFallback(
      path_context.current_input_path, fallback_input_path);
  descriptor.output_path = ResolvePathOrFallback(
      path_context.current_output_path, fallback_output_path);
  const std::filesystem::path input_root_path =
      ResolveRootOrFallback(path_context.input_root_path, descriptor.input_path);
  descriptor.group_label =
      ResolveGroupLabel(input_root_path, descriptor.input_path);
  descriptor.group_index = 1;
  descriptor.group_file_index = 1;
  descriptor.group_file_count = 1;
  descriptor.file_index = 1;
  descriptor.total_files = 1;
  descriptor.input_size_bytes = input_size_bytes;
  return descriptor;
}

auto PrepareSingleFileReporter(
    const std::filesystem::path& fallback_input_path,
    const std::filesystem::path& fallback_output_path,
    const FileCryptoPathContext& path_context, std::uint64_t input_size_bytes,
    file_crypto_internal::ProgressReporter& reporter) -> FileCryptoResult {
  const std::filesystem::path input_path =
      ResolvePathOrFallback(path_context.current_input_path, fallback_input_path);
  const std::filesystem::path output_path = ResolvePathOrFallback(
      path_context.current_output_path, fallback_output_path);
  const std::filesystem::path input_root_path =
      ResolveRootOrFallback(path_context.input_root_path, input_path);
  const std::filesystem::path output_root_path =
      ResolveRootOrFallback(path_context.output_root_path, output_path);
  if (const auto scan_result =
          reporter.BeginScan(input_root_path, output_root_path);
      !scan_result.ok()) {
    return scan_result;
  }
  if (const auto totals_result =
          reporter.SetAggregateTotals(1, input_size_bytes, 1, true);
      !totals_result.ok()) {
    return totals_result;
  }
  if (const auto select_result = reporter.SetCurrentFile(
          BuildSingleFileDescriptor(path_context, fallback_input_path,
                                    fallback_output_path, input_size_bytes),
          0);
      !select_result.ok()) {
    return select_result;
  }
  return {};
}

}  // namespace

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

auto EncryptBytesToFile(std::span<const std::uint8_t> plaintext_bytes,
                        const std::filesystem::path& output_tracer_path,
                        std::string_view passphrase,
                        const FileCryptoPathContext& path_context,
                        const FileCryptoOptions& options) -> FileCryptoResult {
  const auto write_callback =
      [&output_tracer_path](
          std::span<const std::uint8_t> bytes) -> FileCryptoResult {
    return file_crypto_internal::WriteAllBytes(
        output_tracer_path,
        std::vector<std::uint8_t>(bytes.begin(), bytes.end()));
  };
  return EncryptBytesToWriter(plaintext_bytes, write_callback, passphrase,
                              path_context, options);
}

auto EncryptBytesToWriter(std::span<const std::uint8_t> plaintext_bytes,
                          const FileCryptoWriteCallback& write_callback,
                          std::string_view passphrase,
                          const FileCryptoPathContext& path_context,
                          const FileCryptoOptions& options) -> FileCryptoResult {
  if (!write_callback) {
    return file_crypto_internal::MakeError(
        FileCryptoError::kInvalidArgument,
        "Output writer callback is required.");
  }
  if (passphrase.empty()) {
    return file_crypto_internal::MakeError(FileCryptoError::kInvalidArgument,
                                           "Passphrase must not be empty.");
  }

  const std::filesystem::path output_label =
      path_context.current_output_path.empty()
          ? std::filesystem::path("memory_output.tracer")
          : path_context.current_output_path;

  file_crypto_internal::ProgressReporter reporter(FileCryptoOperation::kEncrypt,
                                                  &options);
  const auto prepare_result = PrepareSingleFileReporter(
      path_context.current_input_path, output_label, path_context,
      static_cast<std::uint64_t>(plaintext_bytes.size()), reporter);
  if (!prepare_result.ok()) {
    return prepare_result;
  }

  auto [encrypt_result, encrypted_bytes] =
      file_crypto_internal::EncryptBytesInternal(
          plaintext_bytes, passphrase, options.security_level, &reporter);
  if (!encrypt_result.ok()) {
    if (file_crypto_internal::IsCancelledError(encrypt_result)) {
      (void)reporter.MarkCancelled();
    } else {
      (void)reporter.MarkFailed();
    }
    return encrypt_result;
  }

  if (const auto phase_result =
          reporter.SetPhase(FileCryptoPhase::kWriteOutput, true);
      !phase_result.ok()) {
    return phase_result;
  }
  const auto write_result =
      file_crypto_internal::WriteAllBytes(write_callback, encrypted_bytes);
  if (!write_result.ok()) {
    (void)reporter.MarkFailed();
    return write_result;
  }
  if (const auto complete_result = reporter.MarkCompleted();
      !complete_result.ok()) {
    return complete_result;
  }
  return {};
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

auto DecryptFileToBytes(
    const std::filesystem::path& input_tracer_path, std::string_view passphrase,
    const FileCryptoPathContext& path_context, const FileCryptoOptions& options)
    -> std::pair<FileCryptoResult, std::vector<std::uint8_t>> {
  if (input_tracer_path.empty()) {
    return {file_crypto_internal::MakeError(
                FileCryptoError::kInvalidArgument, "Input path is required."),
            {}};
  }
  if (passphrase.empty()) {
    return {file_crypto_internal::MakeError(
                FileCryptoError::kInvalidArgument,
                "Passphrase must not be empty."),
            {}};
  }

  std::error_code size_error;
  const auto input_size = std::filesystem::file_size(input_tracer_path,
                                                     size_error);
  if (size_error) {
    return {file_crypto_internal::MakeError(FileCryptoError::kInputReadFailed,
                                            "Failed to read input file size."),
            {}};
  }

  file_crypto_internal::ProgressReporter reporter(FileCryptoOperation::kDecrypt,
                                                  &options);
  const auto prepare_result = PrepareSingleFileReporter(
      input_tracer_path, path_context.current_output_path, path_context,
      input_size, reporter);
  if (!prepare_result.ok()) {
    return {prepare_result, {}};
  }

  if (const auto phase_result =
          reporter.SetPhase(FileCryptoPhase::kReadInput, true);
      !phase_result.ok()) {
    return {phase_result, {}};
  }
  auto [read_result, encrypted_bytes] =
      file_crypto_internal::ReadAllBytes(input_tracer_path, &reporter);
  if (!read_result.ok()) {
    if (file_crypto_internal::IsCancelledError(read_result)) {
      (void)reporter.MarkCancelled();
    } else {
      (void)reporter.MarkFailed();
    }
    return {read_result, {}};
  }

  auto [decrypt_result, plaintext_bytes] =
      file_crypto_internal::DecryptBytesInternal(encrypted_bytes, passphrase,
                                                 &reporter);
  if (!decrypt_result.ok()) {
    if (file_crypto_internal::IsCancelledError(decrypt_result)) {
      (void)reporter.MarkCancelled();
    } else {
      (void)reporter.MarkFailed();
    }
    return {decrypt_result, {}};
  }

  if (const auto phase_result =
          reporter.SetPhase(FileCryptoPhase::kWriteOutput, true);
      !phase_result.ok()) {
    return {phase_result, {}};
  }
  if (const auto complete_result = reporter.MarkCompleted();
      !complete_result.ok()) {
    return {complete_result, {}};
  }
  return {{}, std::move(plaintext_bytes)};
}

auto EncryptDirectory(const std::filesystem::path& input_root_path,
                      const std::filesystem::path& output_root_path,
                      std::string_view passphrase,
                      const FileCryptoOptions& options)
    -> FileCryptoBatchResult {
  const file_crypto_internal::DirectoryCryptoExtensions kExtensions{
      .input_extension_lower = ".txt",
      .output_extension_lower = ".tracer",
  };
  return file_crypto_internal::RunDirectoryCrypto(
      FileCryptoOperation::kEncrypt, input_root_path, output_root_path,
      passphrase, kExtensions, options);
}

auto DecryptDirectory(const std::filesystem::path& input_root_path,
                      const std::filesystem::path& output_root_path,
                      std::string_view passphrase,
                      const FileCryptoOptions& options)
    -> FileCryptoBatchResult {
  const file_crypto_internal::DirectoryCryptoExtensions kExtensions{
      .input_extension_lower = ".tracer",
      .output_extension_lower = ".txt",
  };
  return file_crypto_internal::RunDirectoryCrypto(
      FileCryptoOperation::kDecrypt, input_root_path, output_root_path,
      passphrase, kExtensions, options);
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
    metadata_out->version = header.kVersion;
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
