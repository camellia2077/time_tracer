// infrastructure/tests/file_crypto/file_crypto_service_roundtrip_tests.cpp
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>

#include "infrastructure/tests/file_crypto/file_crypto_service_test_internal.hpp"

namespace android_runtime_tests {
namespace {

auto TestEncryptDecryptRoundTrip(int& failures) -> void {
  using namespace file_crypto_tests_internal;

  const RuntimeTestPaths kPaths =
      BuildTempTestPaths("tracer_core_file_crypto_roundtrip_test");
  const auto kInputTxt = kPaths.test_root / "plain.txt";
  const auto kEncrypted = kPaths.test_root / "payload.tracer";
  const auto kRestoredTxt = kPaths.test_root / "restored.txt";
  constexpr std::string_view kPassphrase = "phase0-phase1-passphrase";
  const std::string kPlaintext =
      "y2026\nm02\n0201\n0600 study_math_calculus r alpha\n";

  RemoveTree(kPaths.test_root);
  if (!WriteFileWithParents(kInputTxt, kPlaintext)) {
    ++failures;
    std::cerr << "[FAIL] Failed to write crypto roundtrip input file.\n";
    RemoveTree(kPaths.test_root);
    return;
  }

  const auto kEncryptResult = tracer_core::infrastructure::crypto::EncryptFile(
      kInputTxt, kEncrypted, kPassphrase);
  Expect(kEncryptResult.ok(),
         "EncryptFile should succeed for a valid plaintext input.", failures);
  if (!kEncryptResult.ok()) {
    std::cerr << "[FAIL] Encrypt error: " << kEncryptResult.error_code << " | "
              << kEncryptResult.error_message << '\n';
    RemoveTree(kPaths.test_root);
    return;
  }

  const auto kDecryptResult = tracer_core::infrastructure::crypto::DecryptFile(
      kEncrypted, kRestoredTxt, kPassphrase);
  Expect(kDecryptResult.ok(),
         "DecryptFile should succeed with the correct passphrase.", failures);
  if (!kDecryptResult.ok()) {
    std::cerr << "[FAIL] Decrypt error: " << kDecryptResult.error_code << " | "
              << kDecryptResult.error_message << '\n';
    RemoveTree(kPaths.test_root);
    return;
  }

  const std::string kExpectedPlaintext = ReadTextFile(kInputTxt);
  const std::string kRestored = ReadTextFile(kRestoredTxt);
  Expect(kRestored == kExpectedPlaintext,
         "DecryptFile output must match original plaintext exactly.", failures);

  tracer_core::infrastructure::crypto::TracerFileMetadata metadata{};
  const auto kInspectResult =
      tracer_core::infrastructure::crypto::InspectEncryptedFile(kEncrypted,
                                                                &metadata);
  Expect(kInspectResult.ok(),
         "InspectEncryptedFile should parse v2 .tracer metadata.", failures);
  if (kInspectResult.ok()) {
    Expect(metadata.version == 2, "Encrypted file format version should be 2.",
           failures);
    Expect(metadata.compression_id == 1,
           "Encrypted file compression_id should be zstd(1).", failures);
    Expect(metadata.compression_level == 1,
           "Encrypted file compression_level should default to 1.", failures);
    Expect(metadata.plaintext_size ==
               static_cast<std::uint64_t>(kExpectedPlaintext.size()),
           "Encrypted file plaintext_size should match source byte length.",
           failures);
    Expect(metadata.ciphertext_size > 0,
           "Encrypted file ciphertext size should be non-zero.", failures);
  }

  RemoveTree(kPaths.test_root);
}

auto TestEncryptBytesDecryptToBytesRoundTrip(int& failures) -> void {
  using namespace file_crypto_tests_internal;

  const RuntimeTestPaths kPaths =
      BuildTempTestPaths("tracer_core_file_crypto_bytes_roundtrip_test");
  const auto kEncrypted = kPaths.test_root / "payload_bytes.tracer";
  constexpr std::string_view kPassphrase = "phase0-phase1-passphrase";
  const std::string kPlaintext =
      "y2026\nm03\n0301\n0630 study_math_linear_algebra\n";
  const std::vector<std::uint8_t> kPlaintextBytes(kPlaintext.begin(),
                                                  kPlaintext.end());
  const tracer_core::infrastructure::crypto::FileCryptoPathContext kPathContext{
      .input_root_path = kPaths.test_root / "logical_input",
      .output_root_path = kEncrypted.parent_path(),
      .current_input_path = kPaths.test_root / "logical_input" /
                            "payload.ttpkg",
      .current_output_path = kEncrypted,
  };

  RemoveTree(kPaths.test_root);

  const auto kEncryptResult =
      tracer_core::infrastructure::crypto::EncryptBytesToFile(
          kPlaintextBytes, kEncrypted, kPassphrase, kPathContext);
  Expect(kEncryptResult.ok(),
         "EncryptBytesToFile should succeed for valid plaintext bytes.",
         failures);
  if (!kEncryptResult.ok()) {
    std::cerr << "[FAIL] EncryptBytesToFile error: "
              << kEncryptResult.error_code << " | "
              << kEncryptResult.error_message << '\n';
    RemoveTree(kPaths.test_root);
    return;
  }

  const auto [kDecryptResult, kRestoredBytes] =
      tracer_core::infrastructure::crypto::DecryptFileToBytes(
          kEncrypted, kPassphrase,
          {.input_root_path = kEncrypted.parent_path(),
           .output_root_path = kPaths.test_root / "logical_output",
           .current_input_path = kEncrypted,
           .current_output_path =
               kPaths.test_root / "logical_output" / "restored.ttpkg"});
  Expect(kDecryptResult.ok(),
         "DecryptFileToBytes should succeed with the correct passphrase.",
         failures);
  if (!kDecryptResult.ok()) {
    std::cerr << "[FAIL] DecryptFileToBytes error: "
              << kDecryptResult.error_code << " | "
              << kDecryptResult.error_message << '\n';
    RemoveTree(kPaths.test_root);
    return;
  }

  Expect(std::string(kRestoredBytes.begin(), kRestoredBytes.end()) == kPlaintext,
         "DecryptFileToBytes output must match original plaintext exactly.",
         failures);
  Expect(std::filesystem::exists(kEncrypted),
         "EncryptBytesToFile should still materialize the .tracer artifact.",
         failures);

  RemoveTree(kPaths.test_root);
}

auto TestEncryptBytesWriterDecryptToBytesRoundTrip(int& failures) -> void {
  using namespace file_crypto_tests_internal;

  const RuntimeTestPaths kPaths =
      BuildTempTestPaths("tracer_core_file_crypto_writer_roundtrip_test");
  const auto kEncrypted = kPaths.test_root / "payload_writer.tracer";
  constexpr std::string_view kPassphrase = "phase0-phase1-passphrase";
  const std::string kPlaintext =
      "y2026\nm04\n0401\n0700 study_native_android\n";
  const std::vector<std::uint8_t> kPlaintextBytes(kPlaintext.begin(),
                                                  kPlaintext.end());
  std::size_t emitted_ciphertext_size = 0;

  RemoveTree(kPaths.test_root);

  const auto kEncryptResult =
      tracer_core::infrastructure::crypto::EncryptBytesToWriter(
          kPlaintextBytes,
          [&kEncrypted, &emitted_ciphertext_size](std::span<const std::uint8_t> bytes)
              -> tracer_core::infrastructure::crypto::FileCryptoResult {
            emitted_ciphertext_size = bytes.size();
            std::error_code create_error;
            std::filesystem::create_directories(kEncrypted.parent_path(),
                                                create_error);
            if (create_error) {
              return {
                  .error =
                      tracer_core::infrastructure::crypto::FileCryptoError::
                          kOutputWriteFailed,
                  .error_code = "crypto.output_write_failed",
                  .error_message = "Failed to create writer output directory.",
              };
            }
            std::ofstream encrypted_output(
                kEncrypted, std::ios::binary | std::ios::trunc);
            if (!encrypted_output.is_open()) {
              return {
                  .error =
                      tracer_core::infrastructure::crypto::FileCryptoError::
                          kOutputWriteFailed,
                  .error_code = "crypto.output_write_failed",
                  .error_message = "Failed to open writer output file.",
              };
            }
            encrypted_output.write(
                reinterpret_cast<const char*>(bytes.data()),
                static_cast<std::streamsize>(bytes.size()));
            if (!encrypted_output.good()) {
              return {
                  .error =
                      tracer_core::infrastructure::crypto::FileCryptoError::
                          kOutputWriteFailed,
                  .error_code = "crypto.output_write_failed",
                  .error_message = "Failed to persist writer output file.",
              };
            }
            return {};
          },
          kPassphrase,
          {.input_root_path = kPaths.test_root / "logical_input",
           .output_root_path = kPaths.test_root / "logical_output",
           .current_input_path =
               kPaths.test_root / "logical_input" / "payload.ttpkg",
           .current_output_path =
               kPaths.test_root / "logical_output" / "payload_writer.tracer"});
  Expect(kEncryptResult.ok(),
         "EncryptBytesToWriter should succeed for valid plaintext bytes.",
         failures);
  if (!kEncryptResult.ok()) {
    std::cerr << "[FAIL] EncryptBytesToWriter error: "
              << kEncryptResult.error_code << " | "
              << kEncryptResult.error_message << '\n';
    RemoveTree(kPaths.test_root);
    return;
  }

  Expect(std::filesystem::exists(kEncrypted),
         "EncryptBytesToWriter callback should persist the tracer output.",
         failures);
  Expect(emitted_ciphertext_size > 0,
         "EncryptBytesToWriter should emit non-empty ciphertext bytes.",
         failures);

  const auto [kDecryptResult, kRestoredBytes] =
      tracer_core::infrastructure::crypto::DecryptFileToBytes(
          kEncrypted, kPassphrase);
  Expect(kDecryptResult.ok(),
         "DecryptFileToBytes should roundtrip writer-produced tracer bytes.",
         failures);
  if (!kDecryptResult.ok()) {
    std::cerr << "[FAIL] DecryptFileToBytes(writer) error: "
              << kDecryptResult.error_code << " | "
              << kDecryptResult.error_message << '\n';
    RemoveTree(kPaths.test_root);
    return;
  }

  Expect(std::string(kRestoredBytes.begin(), kRestoredBytes.end()) == kPlaintext,
         "Writer-based encrypted bytes should decrypt back to the original "
         "plaintext.",
         failures);

  RemoveTree(kPaths.test_root);
}

}  // namespace

auto RunFileCryptoRoundtripTests(int& failures) -> void {
  TestEncryptDecryptRoundTrip(failures);
  TestEncryptBytesDecryptToBytesRoundTrip(failures);
  TestEncryptBytesWriterDecryptToBytesRoundTrip(failures);
}

}  // namespace android_runtime_tests
