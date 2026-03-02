// infrastructure/tests/file_crypto/file_crypto_service_failure_tests.cpp
#include <cstddef>
#include <cstdint>
#include <iostream>

#include "infrastructure/tests/file_crypto/file_crypto_service_test_internal.hpp"

namespace android_runtime_tests {
namespace {

auto TestWrongPassphraseRejected(int& failures) -> void {
  using namespace file_crypto_tests_internal;

  const RuntimeTestPaths kPaths =
      BuildTempTestPaths("tracer_core_file_crypto_wrong_passphrase_test");
  const auto kInputTxt = kPaths.test_root / "plain.txt";
  const auto kEncrypted = kPaths.test_root / "payload.tracer";
  const auto kRestoredTxt = kPaths.test_root / "restored.txt";

  RemoveTree(kPaths.test_root);
  if (!WriteFileWithParents(kInputTxt, "y2026\nm02\n0202\n0630 study\n")) {
    ++failures;
    std::cerr << "[FAIL] Failed to write wrong-passphrase input file.\n";
    RemoveTree(kPaths.test_root);
    return;
  }

  const auto kEncryptResult = tracer_core::infrastructure::crypto::EncryptFile(
      kInputTxt, kEncrypted, "correct-passphrase");
  if (!kEncryptResult.ok()) {
    ++failures;
    std::cerr << "[FAIL] Encrypt setup failed: " << kEncryptResult.error_code
              << " | " << kEncryptResult.error_message << '\n';
    RemoveTree(kPaths.test_root);
    return;
  }

  const auto kDecryptResult = tracer_core::infrastructure::crypto::DecryptFile(
      kEncrypted, kRestoredTxt, "wrong-passphrase");
  Expect(!kDecryptResult.ok(),
         "DecryptFile should fail when passphrase is incorrect.", failures);
  if (!kDecryptResult.ok()) {
    Expect(kDecryptResult.error == tracer_core::infrastructure::crypto::
                                       FileCryptoError::kDecryptFailed,
           "Wrong passphrase should map to kDecryptFailed.", failures);
  }

  RemoveTree(kPaths.test_root);
}

auto TestCompressionMetadataMismatchRejected(int& failures) -> void {
  using namespace file_crypto_tests_internal;

  const RuntimeTestPaths kPaths =
      BuildTempTestPaths("tracer_core_file_crypto_mismatch_test");
  const auto kInputTxt = kPaths.test_root / "plain.txt";
  const auto kEncrypted = kPaths.test_root / "payload.tracer";
  const auto kTampered = kPaths.test_root / "payload_tampered.tracer";
  const auto kRestoredTxt = kPaths.test_root / "restored.txt";
  constexpr std::string_view kPassphrase = "phase0-phase1-passphrase";
  constexpr std::size_t kMinimumEncryptedHeaderSize = 80;

  RemoveTree(kPaths.test_root);
  if (!WriteFileWithParents(kInputTxt, "y2026\nm02\n0203\n0700 study\n")) {
    ++failures;
    std::cerr << "[FAIL] Failed to write compression mismatch input file.\n";
    RemoveTree(kPaths.test_root);
    return;
  }

  const auto kEncryptResult = tracer_core::infrastructure::crypto::EncryptFile(
      kInputTxt, kEncrypted, kPassphrase);
  if (!kEncryptResult.ok()) {
    ++failures;
    std::cerr << "[FAIL] Encrypt setup failed: " << kEncryptResult.error_code
              << " | " << kEncryptResult.error_message << '\n';
    RemoveTree(kPaths.test_root);
    return;
  }

  auto bytes = ReadBytes(kEncrypted);
  if (bytes.size() < kMinimumEncryptedHeaderSize) {
    ++failures;
    std::cerr << "[FAIL] Encrypted file is smaller than expected v2 header.\n";
    RemoveTree(kPaths.test_root);
    return;
  }

  constexpr std::size_t kPlaintextSizeOffsetV2 = 60;
  constexpr std::uint64_t kTamperedPlaintextSize = 999999ULL;
  WriteU64LE(bytes, kPlaintextSizeOffsetV2, kTamperedPlaintextSize);
  if (!WriteBytes(kTampered, bytes)) {
    ++failures;
    std::cerr << "[FAIL] Failed to write tampered encrypted file.\n";
    RemoveTree(kPaths.test_root);
    return;
  }

  const auto kDecryptResult = tracer_core::infrastructure::crypto::DecryptFile(
      kTampered, kRestoredTxt, kPassphrase);
  Expect(!kDecryptResult.ok(),
         "DecryptFile should fail when compression metadata is tampered.",
         failures);
  if (!kDecryptResult.ok()) {
    Expect(kDecryptResult.error ==
               tracer_core::infrastructure::crypto::FileCryptoError::
                   kCompressionMetadataMismatch,
           "Tampered plaintext_size should map to "
           "kCompressionMetadataMismatch.",
           failures);
  }

  RemoveTree(kPaths.test_root);
}

}  // namespace

auto RunFileCryptoFailureTests(int& failures) -> void {
  TestWrongPassphraseRejected(failures);
  TestCompressionMetadataMismatchRejected(failures);
}

}  // namespace android_runtime_tests
