// infrastructure/tests/file_crypto/file_crypto_service_roundtrip_tests.cpp
#include <cstdint>
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

}  // namespace

auto RunFileCryptoRoundtripTests(int& failures) -> void {
  TestEncryptDecryptRoundTrip(failures);
}

}  // namespace android_runtime_tests
