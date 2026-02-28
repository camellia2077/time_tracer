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

  const RuntimeTestPaths paths =
      BuildTempTestPaths("tracer_core_file_crypto_android_to_windows_test");
  const auto android_plain_txt =
      ResolveRepoRootForInterop() / "test" / "data" / "2026" / "2026-01.txt";
  const auto interop_tracer = paths.test_root / "android_export.tracer";
  const auto windows_restored_txt = paths.test_root / "windows_restored.txt";
  constexpr std::string_view kPassphrase = "phase3-interop-passphrase";

  RemoveTree(paths.test_root);
  if (!std::filesystem::exists(android_plain_txt)) {
    ++failures;
    std::cerr << "[FAIL] Missing Android-side plaintext fixture: "
              << android_plain_txt.string() << '\n';
    RemoveTree(paths.test_root);
    return;
  }

  const auto android_encrypt_result =
      tracer_core::infrastructure::crypto::EncryptFile(
          android_plain_txt, interop_tracer, kPassphrase);
  Expect(android_encrypt_result.ok(),
         "Android-side export should generate .tracer successfully.", failures);
  if (!android_encrypt_result.ok()) {
    std::cerr << "[FAIL] Android export error: "
              << android_encrypt_result.error_code << " | "
              << android_encrypt_result.error_message << '\n';
    RemoveTree(paths.test_root);
    return;
  }

  const auto windows_decrypt_result =
      tracer_core::infrastructure::crypto::DecryptFile(
          interop_tracer, windows_restored_txt, kPassphrase);
  Expect(windows_decrypt_result.ok(),
         "Windows-side decrypt should accept Android-produced .tracer.",
         failures);
  if (!windows_decrypt_result.ok()) {
    std::cerr << "[FAIL] Windows decrypt error: "
              << windows_decrypt_result.error_code << " | "
              << windows_decrypt_result.error_message << '\n';
    RemoveTree(paths.test_root);
    return;
  }

  const std::string restored = ReadTextFile(windows_restored_txt);
  const std::string expected_plaintext = ReadTextFile(android_plain_txt);
  Expect(restored == expected_plaintext,
         "Android->Windows interop plaintext should round-trip unchanged.",
         failures);

  RemoveTree(paths.test_root);
}

auto TestWindowsToAndroidCryptoImportInterop(int& failures) -> void {
  using namespace file_crypto_tests_internal;

  const RuntimeTestPaths paths =
      BuildTempTestPaths("tracer_core_file_crypto_windows_to_android_test");
  const auto windows_plain_txt =
      ResolveRepoRootForInterop() / "test" / "data" / "2026" / "2026-01.txt";
  const auto interop_tracer = paths.test_root / "windows_export.tracer";
  const auto android_import_txt = paths.test_root / "android_import.txt";
  const auto converter_config_toml = ResolveRepoRootForInterop() / "apps" /
                                     "tracer_core" / "config" / "converter" /
                                     "interval_processor_config.toml";
  constexpr std::string_view kPassphrase = "phase3-interop-passphrase";

  RemoveTree(paths.test_root);
  if (!std::filesystem::exists(windows_plain_txt)) {
    ++failures;
    std::cerr << "[FAIL] Missing Windows-side plaintext fixture: "
              << windows_plain_txt.string() << '\n';
    RemoveTree(paths.test_root);
    return;
  }

  const auto windows_encrypt_result =
      tracer_core::infrastructure::crypto::EncryptFile(
          windows_plain_txt, interop_tracer, kPassphrase);
  Expect(windows_encrypt_result.ok(),
         "Windows-side export should generate .tracer successfully.", failures);
  if (!windows_encrypt_result.ok()) {
    std::cerr << "[FAIL] Windows export error: "
              << windows_encrypt_result.error_code << " | "
              << windows_encrypt_result.error_message << '\n';
    RemoveTree(paths.test_root);
    return;
  }

  const auto android_decrypt_result =
      tracer_core::infrastructure::crypto::DecryptFile(
          interop_tracer, android_import_txt, kPassphrase);
  Expect(android_decrypt_result.ok(),
         "Android-side decrypt should accept Windows-produced .tracer.",
         failures);
  if (!android_decrypt_result.ok()) {
    std::cerr << "[FAIL] Android decrypt error: "
              << android_decrypt_result.error_code << " | "
              << android_decrypt_result.error_message << '\n';
    RemoveTree(paths.test_root);
    return;
  }
  const std::string source_plaintext = ReadTextFile(windows_plain_txt);
  const std::string imported_plaintext = ReadTextFile(android_import_txt);
  Expect(imported_plaintext == source_plaintext,
         "Windows->Android decrypted TXT should match the Windows source TXT.",
         failures);

  try {
    const auto runtime_request =
        BuildRuntimeRequest(paths, converter_config_toml);
    auto runtime =
        infrastructure::bootstrap::BuildAndroidRuntime(runtime_request);
    if (!runtime.core_api) {
      ++failures;
      std::cerr << "[FAIL] BuildAndroidRuntime should provide a valid core "
                   "API for interop import.\n";
      RemoveTree(paths.test_root);
      return;
    }

    const auto structure_ack = runtime.core_api->RunValidateStructure(
        {.input_path = android_import_txt.string()});
    Expect(structure_ack.ok,
           "Android import pre-check: structure validation should pass.",
           failures);
    if (!structure_ack.ok) {
      std::cerr << "[FAIL] Android structure validation failed: "
                << structure_ack.error_message << '\n';
    }

    const auto logic_ack = runtime.core_api->RunValidateLogic(
        {.input_path = android_import_txt.string(),
         .date_check_mode = DateCheckMode::kNone});
    Expect(logic_ack.ok,
           "Android import pre-check: logic validation should pass.", failures);
    if (!logic_ack.ok) {
      std::cerr << "[FAIL] Android logic validation failed: "
                << logic_ack.error_message << '\n';
    }

    tracer_core::core::dto::IngestRequest ingest_request;
    ingest_request.input_path = android_import_txt.string();
    ingest_request.date_check_mode = DateCheckMode::kNone;
    ingest_request.ingest_mode = IngestMode::kSingleTxtReplaceMonth;
    const auto ingest_ack = runtime.core_api->RunIngest(ingest_request);
    Expect(ingest_ack.ok,
           "Android import should ingest TXT decrypted from Windows .tracer.",
           failures);
    if (!ingest_ack.ok) {
      std::cerr << "[FAIL] Android ingest failed: " << ingest_ack.error_message
                << '\n';
      RemoveTree(paths.test_root);
      return;
    }

    sqlite3* database = nullptr;
    if (sqlite3_open(paths.db_path.string().c_str(), &database) != SQLITE_OK ||
        database == nullptr) {
      ++failures;
      std::cerr << "[FAIL] sqlite3_open should succeed for Android import DB "
                   "verification.\n";
      if (database != nullptr) {
        sqlite3_close(database);
      }
      RemoveTree(paths.test_root);
      return;
    }

    const auto imported_count = QueryCount(
        database, "SELECT COUNT(*) FROM time_records WHERE date='2026-01-01';");
    sqlite3_close(database);

    if (!imported_count.has_value()) {
      ++failures;
      std::cerr << "[FAIL] Failed to query imported rows for interop "
                   "verification.\n";
    } else {
      Expect(*imported_count > 0,
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

  RemoveTree(paths.test_root);
}

}  // namespace

auto RunFileCryptoInteropTests(int& failures) -> void {
  TestAndroidToWindowsCryptoInterop(failures);
  TestWindowsToAndroidCryptoImportInterop(failures);
}

}  // namespace android_runtime_tests
