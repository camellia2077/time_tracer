// infrastructure/tests/file_crypto/file_crypto_service_roundtrip_tests.cpp
#include <cstdint>
#include <iostream>
#include <string>

#include "infrastructure/tests/file_crypto/file_crypto_service_test_internal.hpp"

namespace android_runtime_tests {
namespace {

auto TestEncryptDecryptRoundTrip(int& failures) -> void {
  using namespace file_crypto_tests_internal;

  const RuntimeTestPaths paths =
      BuildTempTestPaths("tracer_core_file_crypto_roundtrip_test");
  const auto input_txt = paths.test_root / "plain.txt";
  const auto encrypted = paths.test_root / "payload.tracer";
  const auto restored_txt = paths.test_root / "restored.txt";
  constexpr std::string_view kPassphrase = "phase0-phase1-passphrase";
  const std::string kPlaintext =
      "y2026\nm02\n0201\n0600 study_math_calculus r alpha\n";

  RemoveTree(paths.test_root);
  if (!WriteFileWithParents(input_txt, kPlaintext)) {
    ++failures;
    std::cerr << "[FAIL] Failed to write crypto roundtrip input file.\n";
    RemoveTree(paths.test_root);
    return;
  }

  const auto encrypt_result = tracer_core::infrastructure::crypto::EncryptFile(
      input_txt, encrypted, kPassphrase);
  Expect(encrypt_result.ok(),
         "EncryptFile should succeed for a valid plaintext input.", failures);
  if (!encrypt_result.ok()) {
    std::cerr << "[FAIL] Encrypt error: " << encrypt_result.error_code << " | "
              << encrypt_result.error_message << '\n';
    RemoveTree(paths.test_root);
    return;
  }

  const auto decrypt_result = tracer_core::infrastructure::crypto::DecryptFile(
      encrypted, restored_txt, kPassphrase);
  Expect(decrypt_result.ok(),
         "DecryptFile should succeed with the correct passphrase.", failures);
  if (!decrypt_result.ok()) {
    std::cerr << "[FAIL] Decrypt error: " << decrypt_result.error_code << " | "
              << decrypt_result.error_message << '\n';
    RemoveTree(paths.test_root);
    return;
  }

  const std::string expected_plaintext = ReadTextFile(input_txt);
  const std::string restored = ReadTextFile(restored_txt);
  Expect(restored == expected_plaintext,
         "DecryptFile output must match original plaintext exactly.", failures);

  tracer_core::infrastructure::crypto::TracerFileMetadata metadata{};
  const auto inspect_result =
      tracer_core::infrastructure::crypto::InspectEncryptedFile(encrypted,
                                                                &metadata);
  Expect(inspect_result.ok(),
         "InspectEncryptedFile should parse v2 .tracer metadata.", failures);
  if (inspect_result.ok()) {
    Expect(metadata.version == 2, "Encrypted file format version should be 2.",
           failures);
    Expect(metadata.compression_id == 1,
           "Encrypted file compression_id should be zstd(1).", failures);
    Expect(metadata.compression_level == 1,
           "Encrypted file compression_level should default to 1.", failures);
    Expect(metadata.plaintext_size ==
               static_cast<std::uint64_t>(expected_plaintext.size()),
           "Encrypted file plaintext_size should match source byte length.",
           failures);
    Expect(metadata.ciphertext_size > 0,
           "Encrypted file ciphertext size should be non-zero.", failures);
  }

  RemoveTree(paths.test_root);
}

}  // namespace

auto RunFileCryptoRoundtripTests(int& failures) -> void {
  TestEncryptDecryptRoundTrip(failures);
}

}  // namespace android_runtime_tests
