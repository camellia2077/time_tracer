// infrastructure/tests/file_crypto/file_crypto_service_interop_tests.cpp
#include <sqlite3.h>

#include <exception>
#include <iostream>
#include <string>

#include "infrastructure/tests/file_crypto/file_crypto_service_test_internal.hpp"

namespace android_runtime_tests {
namespace {

auto TestAndroidToWindowsCryptoInterop(int& failures) -> void {
  using namespace file_crypto_tests_internal;

  const RuntimeTestPaths kPaths =
      BuildTempTestPaths("tracer_core_file_crypto_android_to_windows_test");
  const auto kAndroidPlainTxt =
      ResolveRepoRootForInterop() / "test" / "data" / "2026" / "2026-01.txt";
  const auto kInteropTracer = kPaths.test_root / "android_export.tracer";
  const auto kWindowsRestoredTxt = kPaths.test_root / "windows_restored.txt";
  constexpr std::string_view kPassphrase = "phase3-interop-passphrase";

  RemoveTree(kPaths.test_root);
  if (!std::filesystem::exists(kAndroidPlainTxt)) {
    ++failures;
    std::cerr << "[FAIL] Missing Android-side plaintext fixture: "
              << kAndroidPlainTxt.string() << '\n';
    RemoveTree(kPaths.test_root);
    return;
  }

  const auto kAndroidEncryptResult =
      tracer_core::infrastructure::crypto::EncryptFile(
          kAndroidPlainTxt, kInteropTracer, kPassphrase);
  Expect(kAndroidEncryptResult.ok(),
         "Android-side export should generate .tracer successfully.", failures);
  if (!kAndroidEncryptResult.ok()) {
    std::cerr << "[FAIL] Android export error: "
              << kAndroidEncryptResult.error_code << " | "
              << kAndroidEncryptResult.error_message << '\n';
    RemoveTree(kPaths.test_root);
    return;
  }

  const auto kWindowsDecryptResult =
      tracer_core::infrastructure::crypto::DecryptFile(
          kInteropTracer, kWindowsRestoredTxt, kPassphrase);
  Expect(kWindowsDecryptResult.ok(),
         "Windows-side decrypt should accept Android-produced .tracer.",
         failures);
  if (!kWindowsDecryptResult.ok()) {
    std::cerr << "[FAIL] Windows decrypt error: "
              << kWindowsDecryptResult.error_code << " | "
              << kWindowsDecryptResult.error_message << '\n';
    RemoveTree(kPaths.test_root);
    return;
  }

  const std::string kRestored = ReadTextFile(kWindowsRestoredTxt);
  const std::string kExpectedPlaintext = ReadTextFile(kAndroidPlainTxt);
  Expect(kRestored == kExpectedPlaintext,
         "Android->Windows interop plaintext should round-trip unchanged.",
         failures);

  RemoveTree(kPaths.test_root);
}

auto TestWindowsToAndroidCryptoImportInterop(int& failures) -> void {
  using namespace file_crypto_tests_internal;

  const RuntimeTestPaths kPaths =
      BuildTempTestPaths("tracer_core_file_crypto_windows_to_android_test");
  const auto kWindowsPlainTxt =
      ResolveRepoRootForInterop() / "test" / "data" / "2026" / "2026-01.txt";
  const auto kInteropTracer = kPaths.test_root / "windows_export.tracer";
  const auto kAndroidImportTxt = kPaths.test_root / "android_import.txt";
  const auto kConverterConfigToml = ResolveRepoRootForInterop() / "assets" /
                                    "tracer_core" / "config" / "converter" /
                                    "interval_processor_config.toml";
  constexpr std::string_view kPassphrase = "phase3-interop-passphrase";

  RemoveTree(kPaths.test_root);
  if (!std::filesystem::exists(kWindowsPlainTxt)) {
    ++failures;
    std::cerr << "[FAIL] Missing Windows-side plaintext fixture: "
              << kWindowsPlainTxt.string() << '\n';
    RemoveTree(kPaths.test_root);
    return;
  }

  const auto kWindowsEncryptResult =
      tracer_core::infrastructure::crypto::EncryptFile(
          kWindowsPlainTxt, kInteropTracer, kPassphrase);
  Expect(kWindowsEncryptResult.ok(),
         "Windows-side export should generate .tracer successfully.", failures);
  if (!kWindowsEncryptResult.ok()) {
    std::cerr << "[FAIL] Windows export error: "
              << kWindowsEncryptResult.error_code << " | "
              << kWindowsEncryptResult.error_message << '\n';
    RemoveTree(kPaths.test_root);
    return;
  }

  const auto kAndroidDecryptResult =
      tracer_core::infrastructure::crypto::DecryptFile(
          kInteropTracer, kAndroidImportTxt, kPassphrase);
  Expect(kAndroidDecryptResult.ok(),
         "Android-side decrypt should accept Windows-produced .tracer.",
         failures);
  if (!kAndroidDecryptResult.ok()) {
    std::cerr << "[FAIL] Android decrypt error: "
              << kAndroidDecryptResult.error_code << " | "
              << kAndroidDecryptResult.error_message << '\n';
    RemoveTree(kPaths.test_root);
    return;
  }
  const std::string kSourcePlaintext = ReadTextFile(kWindowsPlainTxt);
  const std::string kImportedPlaintext = ReadTextFile(kAndroidImportTxt);
  Expect(kImportedPlaintext == kSourcePlaintext,
         "Windows->Android decrypted TXT should match the Windows source TXT.",
         failures);

  try {
    const auto kRuntimeRequest =
        BuildRuntimeRequest(kPaths, kConverterConfigToml);
    auto runtime =
        infrastructure::bootstrap::BuildAndroidRuntime(kRuntimeRequest);
    if (!runtime.runtime_api) {
      ++failures;
      std::cerr << "[FAIL] BuildAndroidRuntime should provide a valid core "
                   "runtime API for interop import.\n";
      RemoveTree(kPaths.test_root);
      return;
    }

    const auto kStructureAck =
        runtime.runtime_api->pipeline().RunValidateStructure(
            {.input_path = kAndroidImportTxt.string()});
    Expect(kStructureAck.ok,
           "Android import pre-check: structure validation should pass.",
           failures);
    if (!kStructureAck.ok) {
      std::cerr << "[FAIL] Android structure validation failed: "
                << kStructureAck.error_message << '\n';
    }

    const auto kLogicAck = runtime.runtime_api->pipeline().RunValidateLogic(
        {.input_path = kAndroidImportTxt.string(),
         .date_check_mode = DateCheckMode::kNone});
    Expect(kLogicAck.ok,
           "Android import pre-check: logic validation should pass.", failures);
    if (!kLogicAck.ok) {
      std::cerr << "[FAIL] Android logic validation failed: "
                << kLogicAck.error_message << '\n';
    }

    tracer_core::core::dto::IngestRequest ingest_request;
    ingest_request.input_path = kAndroidImportTxt.string();
    ingest_request.date_check_mode = DateCheckMode::kNone;
    ingest_request.ingest_mode = IngestMode::kSingleTxtReplaceMonth;
    const auto kIngestAck =
        runtime.runtime_api->pipeline().RunIngest(ingest_request);
    Expect(kIngestAck.ok,
           "Android import should ingest TXT decrypted from Windows .tracer.",
           failures);
    if (!kIngestAck.ok) {
      std::cerr << "[FAIL] Android ingest failed: " << kIngestAck.error_message
                << '\n';
      RemoveTree(kPaths.test_root);
      return;
    }

    sqlite3* database = nullptr;
    if (sqlite3_open(kPaths.db_path.string().c_str(), &database) != SQLITE_OK ||
        database == nullptr) {
      ++failures;
      std::cerr << "[FAIL] sqlite3_open should succeed for Android import DB "
                   "verification.\n";
      if (database != nullptr) {
        sqlite3_close(database);
      }
      RemoveTree(kPaths.test_root);
      return;
    }

    const auto kImportedCount = QueryCount(
        database, "SELECT COUNT(*) FROM time_records WHERE date='2026-01-01';");
    sqlite3_close(database);

    if (!kImportedCount.has_value()) {
      ++failures;
      std::cerr << "[FAIL] Failed to query imported rows for interop "
                   "verification.\n";
    } else {
      Expect(*kImportedCount > 0,
             "Android import should persist records decrypted from Windows "
             ".tracer.",
             failures);
    }
  } catch (const std::exception& exception) {
    ++failures;
    std::cerr << "[FAIL] Windows->Android interop test threw exception: "
              << exception.what() << '\n';
  } catch (...) {
    ++failures;
    std::cerr << "[FAIL] Windows->Android interop test threw non-standard "
                 "exception.\n";
  }

  RemoveTree(kPaths.test_root);
}

}  // namespace

auto RunFileCryptoInteropTests(int& failures) -> void {
  TestAndroidToWindowsCryptoInterop(failures);
  TestWindowsToAndroidCryptoImportInterop(failures);
}

}  // namespace android_runtime_tests
