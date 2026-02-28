// infrastructure/tests/file_crypto/file_crypto_service_progress_tests.cpp
#include <chrono>
#include <cstdint>
#include <iostream>
#include <vector>

#include "infrastructure/tests/file_crypto/file_crypto_service_test_internal.hpp"

namespace android_runtime_tests {
namespace {

auto TestBatchEncryptProgressSnapshot(int& failures) -> void {
  using namespace file_crypto_tests_internal;

  const RuntimeTestPaths paths =
      BuildTempTestPaths("tracer_core_file_crypto_batch_progress_test");
  const auto input_root = paths.test_root / "input";
  const auto output_root = paths.test_root / "encrypted";
  constexpr std::string_view kPassphrase = "batch-progress-passphrase";

  RemoveTree(paths.test_root);
  const bool seeded = WriteFileWithParents(input_root / "2025" / "2025-01.txt",
                                           "y2025\nm01\n0101\n0600 study\n") &&
                      WriteFileWithParents(input_root / "2025" / "2025-02.txt",
                                           "y2025\nm02\n0201\n0630 work\n") &&
                      WriteFileWithParents(input_root / "2026" / "2026-01.txt",
                                           "y2026\nm01\n0101\n0700 run\n");
  if (!seeded) {
    ++failures;
    std::cerr << "[FAIL] Failed to seed batch progress plaintext files.\n";
    RemoveTree(paths.test_root);
    return;
  }

  using tracer_core::infrastructure::crypto::FileCryptoControl;
  using tracer_core::infrastructure::crypto::FileCryptoOptions;
  using tracer_core::infrastructure::crypto::FileCryptoPhase;
  using tracer_core::infrastructure::crypto::FileCryptoProgressSnapshot;

  std::vector<FileCryptoProgressSnapshot> snapshots;
  FileCryptoOptions options{};
  options.progress_min_interval = std::chrono::milliseconds(0);
  options.progress_min_bytes_delta = 1;
  options.progress_callback =
      [&](const FileCryptoProgressSnapshot& snapshot) -> FileCryptoControl {
    snapshots.push_back(snapshot);
    return FileCryptoControl::kContinue;
  };

  const auto batch_result =
      tracer_core::infrastructure::crypto::EncryptDirectory(
          input_root, output_root, kPassphrase, options);
  Expect(batch_result.ok(), "EncryptDirectory should succeed for valid batch.",
         failures);
  Expect(batch_result.total_files == 3,
         "EncryptDirectory total_files should equal scanned txt files.",
         failures);
  Expect(batch_result.succeeded_files == 3,
         "EncryptDirectory should report all files as succeeded.", failures);
  Expect(batch_result.failed_files == 0,
         "EncryptDirectory failed_files should be zero on success.", failures);

  const auto encrypted_count = CountFilesByExtension(output_root, ".tracer");
  Expect(encrypted_count == 3,
         "EncryptDirectory should output one .tracer per source txt.",
         failures);

  BatchProgressExpectationState state{};
  for (const auto& snapshot : snapshots) {
    UpdateBatchProgressExpectationState(state, snapshot);
  }

  Expect(!snapshots.empty(),
         "Progress callback should receive at least one snapshot.", failures);
  Expect(state.has_scan, "Progress snapshots should include scan phase.",
         failures);
  Expect(state.has_read_input,
         "Progress snapshots should include read-input phase.", failures);
  Expect(state.has_completed,
         "Progress snapshots should include completed phase.", failures);
  Expect(state.has_group_count,
         "Progress snapshots should include discovered top-level group count.",
         failures);
  Expect(state.overall_monotonic,
         "overall_done_bytes should be monotonic non-decreasing.", failures);
  Expect(state.remaining_bytes_consistent,
         "remaining_bytes should equal overall_total_bytes-overall_done_bytes.",
         failures);
  Expect(state.eta_consistent,
         "eta_seconds should be consistent with speed_bytes_per_sec and "
         "remaining_bytes.",
         failures);
  Expect(state.completed_eta_cleared,
         "Completed snapshot should clear remaining_bytes and eta_seconds.",
         failures);
  if (!snapshots.empty()) {
    Expect(snapshots.back().phase == FileCryptoPhase::kCompleted,
           "Final progress snapshot should be completed.", failures);
  }

  RemoveTree(paths.test_root);
}

auto TestBatchDecryptCancellation(int& failures) -> void {
  using namespace file_crypto_tests_internal;

  const RuntimeTestPaths paths =
      BuildTempTestPaths("tracer_core_file_crypto_batch_cancel_test");
  const auto plaintext_root = paths.test_root / "plain";
  const auto encrypted_root = paths.test_root / "encrypted";
  const auto decrypted_root = paths.test_root / "decrypted";
  constexpr std::string_view kPassphrase = "batch-cancel-passphrase";

  RemoveTree(paths.test_root);
  const bool seeded =
      WriteFileWithParents(plaintext_root / "2025" / "2025-01.txt",
                           "y2025\nm01\n0101\n0600 study\n") &&
      WriteFileWithParents(plaintext_root / "2025" / "2025-02.txt",
                           "y2025\nm02\n0202\n0630 work\n") &&
      WriteFileWithParents(plaintext_root / "2026" / "2026-01.txt",
                           "y2026\nm01\n0103\n0700 run\n");
  if (!seeded) {
    ++failures;
    std::cerr << "[FAIL] Failed to seed batch cancel plaintext files.\n";
    RemoveTree(paths.test_root);
    return;
  }

  const auto encrypt_result =
      tracer_core::infrastructure::crypto::EncryptDirectory(
          plaintext_root, encrypted_root, kPassphrase);
  if (!encrypt_result.ok()) {
    ++failures;
    std::cerr << "[FAIL] Batch decrypt cancellation setup encrypt failed: "
              << encrypt_result.status.error_code << " | "
              << encrypt_result.status.error_message << '\n';
    RemoveTree(paths.test_root);
    return;
  }

  using tracer_core::infrastructure::crypto::FileCryptoControl;
  using tracer_core::infrastructure::crypto::FileCryptoError;
  using tracer_core::infrastructure::crypto::FileCryptoOptions;
  using tracer_core::infrastructure::crypto::FileCryptoPhase;
  using tracer_core::infrastructure::crypto::FileCryptoProgressSnapshot;

  bool cancel_requested = false;
  FileCryptoOptions options{};
  options.progress_min_interval = std::chrono::milliseconds(0);
  options.progress_min_bytes_delta = 1;
  options.progress_callback =
      [&](const FileCryptoProgressSnapshot& snapshot) -> FileCryptoControl {
    if (!cancel_requested && snapshot.phase == FileCryptoPhase::kReadInput &&
        snapshot.current_file_done_bytes > 0) {
      cancel_requested = true;
      return FileCryptoControl::kCancel;
    }
    return FileCryptoControl::kContinue;
  };

  const auto decrypt_result =
      tracer_core::infrastructure::crypto::DecryptDirectory(
          encrypted_root, decrypted_root, kPassphrase, options);
  Expect(!decrypt_result.ok(),
         "DecryptDirectory should fail when callback requests cancellation.",
         failures);
  Expect(decrypt_result.cancelled,
         "DecryptDirectory should mark cancelled=true when user cancels.",
         failures);
  Expect(decrypt_result.status.error == FileCryptoError::kCancelled,
         "DecryptDirectory cancellation should map to kCancelled.", failures);
  Expect(cancel_requested,
         "Cancellation callback branch should be triggered at least once.",
         failures);
  const auto decrypted_count = CountFilesByExtension(decrypted_root, ".txt");
  Expect(decrypted_count < decrypt_result.total_files,
         "Cancelled batch decrypt should not finish all files.", failures);

  RemoveTree(paths.test_root);
}

auto TestSingleFileEncryptCancelToken(int& failures) -> void {
  using namespace file_crypto_tests_internal;

  const RuntimeTestPaths paths =
      BuildTempTestPaths("tracer_core_file_crypto_cancel_token_test");
  const auto input_txt = paths.test_root / "plain.txt";
  const auto encrypted = paths.test_root / "payload.tracer";
  constexpr std::string_view kPassphrase = "single-cancel-token-passphrase";

  RemoveTree(paths.test_root);
  if (!WriteFileWithParents(input_txt,
                            "y2026\nm02\n0201\n0600 study_math_calculus\n")) {
    ++failures;
    std::cerr << "[FAIL] Failed to seed cancel token input file.\n";
    RemoveTree(paths.test_root);
    return;
  }

  using tracer_core::infrastructure::crypto::FileCryptoCancelToken;
  using tracer_core::infrastructure::crypto::FileCryptoControl;
  using tracer_core::infrastructure::crypto::FileCryptoError;
  using tracer_core::infrastructure::crypto::FileCryptoOptions;
  using tracer_core::infrastructure::crypto::FileCryptoPhase;
  using tracer_core::infrastructure::crypto::FileCryptoProgressSnapshot;

  FileCryptoCancelToken cancel_token{};
  bool cancel_armed = false;
  FileCryptoOptions options{};
  options.cancel_token = &cancel_token;
  options.progress_min_interval = std::chrono::milliseconds(0);
  options.progress_min_bytes_delta = 1;
  options.progress_callback =
      [&](const FileCryptoProgressSnapshot& snapshot) -> FileCryptoControl {
    if (!cancel_armed && snapshot.phase == FileCryptoPhase::kReadInput &&
        snapshot.current_file_done_bytes > 0) {
      cancel_token.RequestCancel();
      cancel_armed = true;
    }
    return FileCryptoControl::kContinue;
  };

  const auto encrypt_result = tracer_core::infrastructure::crypto::EncryptFile(
      input_txt, encrypted, kPassphrase, options);
  Expect(!encrypt_result.ok(),
         "EncryptFile should fail when cancel token is triggered.", failures);
  Expect(encrypt_result.error == FileCryptoError::kCancelled,
         "EncryptFile cancel token should map to kCancelled.", failures);
  Expect(cancel_armed, "Cancel token path should be exercised.", failures);

  RemoveTree(paths.test_root);
}

}  // namespace

auto RunFileCryptoProgressTests(int& failures) -> void {
  TestBatchEncryptProgressSnapshot(failures);
  TestBatchDecryptCancellation(failures);
  TestSingleFileEncryptCancelToken(failures);
}

}  // namespace android_runtime_tests
