// infrastructure/tests/file_crypto/file_crypto_service_failure_tests.cpp
#include <cstddef>
#include <cstdint>
#include <iostream>

#include "infrastructure/tests/file_crypto/file_crypto_service_test_internal.hpp"

namespace android_runtime_tests {
namespace {

auto TestWrongPassphraseRejected(int& failures) -> void {
  using namespace file_crypto_tests_internal;

  const RuntimeTestPaths paths =
      BuildTempTestPaths("tracer_core_file_crypto_wrong_passphrase_test");
  const auto input_txt = paths.test_root / "plain.txt";
  const auto encrypted = paths.test_root / "payload.tracer";
  const auto restored_txt = paths.test_root / "restored.txt";

  RemoveTree(paths.test_root);
  if (!WriteFileWithParents(input_txt, "y2026\nm02\n0202\n0630 study\n")) {
    ++failures;
    std::cerr << "[FAIL] Failed to write wrong-passphrase input file.\n";
    RemoveTree(paths.test_root);
    return;
  }

  const auto encrypt_result = tracer_core::infrastructure::crypto::EncryptFile(
      input_txt, encrypted, "correct-passphrase");
  if (!encrypt_result.ok()) {
    ++failures;
    std::cerr << "[FAIL] Encrypt setup failed: " << encrypt_result.error_code
              << " | " << encrypt_result.error_message << '\n';
    RemoveTree(paths.test_root);
    return;
  }

  const auto decrypt_result = tracer_core::infrastructure::crypto::DecryptFile(
      encrypted, restored_txt, "wrong-passphrase");
  Expect(!decrypt_result.ok(),
         "DecryptFile should fail when passphrase is incorrect.", failures);
  if (!decrypt_result.ok()) {
    Expect(decrypt_result.error == tracer_core::infrastructure::crypto::
                                       FileCryptoError::kDecryptFailed,
           "Wrong passphrase should map to kDecryptFailed.", failures);
  }

  RemoveTree(paths.test_root);
}

auto TestCompressionMetadataMismatchRejected(int& failures) -> void {
  using namespace file_crypto_tests_internal;

  const RuntimeTestPaths paths =
      BuildTempTestPaths("tracer_core_file_crypto_mismatch_test");
  const auto input_txt = paths.test_root / "plain.txt";
  const auto encrypted = paths.test_root / "payload.tracer";
  const auto tampered = paths.test_root / "payload_tampered.tracer";
  const auto restored_txt = paths.test_root / "restored.txt";
  constexpr std::string_view kPassphrase = "phase0-phase1-passphrase";
  constexpr std::size_t kMinimumEncryptedHeaderSize = 80;

  RemoveTree(paths.test_root);
  if (!WriteFileWithParents(input_txt, "y2026\nm02\n0203\n0700 study\n")) {
    ++failures;
    std::cerr << "[FAIL] Failed to write compression mismatch input file.\n";
    RemoveTree(paths.test_root);
    return;
  }

  const auto encrypt_result = tracer_core::infrastructure::crypto::EncryptFile(
      input_txt, encrypted, kPassphrase);
  if (!encrypt_result.ok()) {
    ++failures;
    std::cerr << "[FAIL] Encrypt setup failed: " << encrypt_result.error_code
              << " | " << encrypt_result.error_message << '\n';
    RemoveTree(paths.test_root);
    return;
  }

  auto bytes = ReadBytes(encrypted);
  if (bytes.size() < kMinimumEncryptedHeaderSize) {
    ++failures;
    std::cerr << "[FAIL] Encrypted file is smaller than expected v2 header.\n";
    RemoveTree(paths.test_root);
    return;
  }

  constexpr std::size_t kPlaintextSizeOffsetV2 = 60;
  constexpr std::uint64_t kTamperedPlaintextSize = 999999ULL;
  WriteU64LE(bytes, kPlaintextSizeOffsetV2, kTamperedPlaintextSize);
  if (!WriteBytes(tampered, bytes)) {
    ++failures;
    std::cerr << "[FAIL] Failed to write tampered encrypted file.\n";
    RemoveTree(paths.test_root);
    return;
  }

  const auto decrypt_result = tracer_core::infrastructure::crypto::DecryptFile(
      tampered, restored_txt, kPassphrase);
  Expect(!decrypt_result.ok(),
         "DecryptFile should fail when compression metadata is tampered.",
         failures);
  if (!decrypt_result.ok()) {
    Expect(decrypt_result.error ==
               tracer_core::infrastructure::crypto::FileCryptoError::
                   kCompressionMetadataMismatch,
           "Tampered plaintext_size should map to "
           "kCompressionMetadataMismatch.",
           failures);
  }

  RemoveTree(paths.test_root);
}

}  // namespace

auto RunFileCryptoFailureTests(int& failures) -> void {
  TestWrongPassphraseRejected(failures);
  TestCompressionMetadataMismatchRejected(failures);
}

}  // namespace android_runtime_tests
