// infrastructure/tests/file_crypto/file_crypto_service_tracer_exchange_import_tests.cpp
#include <string>
#include <string_view>

#include "infrastructure/tests/file_crypto/file_crypto_service_tracer_exchange_test_support.hpp"

namespace android_runtime_tests {
namespace {

using namespace file_crypto_tests_internal;
using namespace tracer_exchange_tests_internal;

auto TestTracerExchangeImportCanonicalizesLegacyText(int& failures) -> void {
  constexpr std::string_view kPassphrase = "phase3-tracer-exchange-passphrase";
  const RuntimeTestPaths paths = BuildTempTestPaths(
      "tracer_core_tracer_exchange_import_canonical_text_test");
  const fs::path config_root = paths.test_root / "config";
  const fs::path main_config_path =
      config_root / "converter" / "interval_processor_config.toml";
  const fs::path package_path = paths.test_root / "package" / "legacy.ttpkg";
  const fs::path tracer_path = paths.test_root / "package" / "legacy.tracer";
  const std::string legacy_payload =
      "\xEF\xBB\xBFy2025\r\nm01\r\n0101\r\n0600 study\r\n";
  const std::string legacy_main =
      "\xEF\xBB\xBFremark_prefix = \"r\"\r\n";
  const std::string legacy_alias =
      "\xEF\xBB\xBF[aliases]\r\nstudy = \"study\"\r\n";
  const std::string legacy_duration =
      "\xEF\xBB\xBF[duration_rules]\r\n";

  if (!PrepareRuntimeFixture(paths, config_root, failures)) {
    return;
  }
  if (!WriteEncryptedTracerFromEntries(
          package_path, tracer_path,
          BuildValidPackageEntries(
              {{.relative_path = "payload/2025/2025-01.txt",
                .text = legacy_payload}},
              legacy_main, legacy_alias, legacy_duration),
          kPassphrase, failures)) {
    RemoveTree(paths.test_root);
    return;
  }

  auto runtime = BuildTracerExchangeRuntime(paths, main_config_path, failures);
  if (!runtime.has_value()) {
    return;
  }

  const fs::path active_text_root = paths.test_root / "input" / "full";
  const fs::path runtime_work_root = paths.test_root / "work";
  const auto result =
      runtime->runtime_api->tracer_exchange().RunTracerExchangeImport({
      .input_tracer_path = tracer_path,
      .active_text_root_path = active_text_root,
      .active_converter_main_config_path = main_config_path,
      .runtime_work_root = runtime_work_root,
      .passphrase = std::string(kPassphrase),
  });
  if (!result.ok) {
    ++failures;
    std::cerr << "[FAIL] Legacy text import should succeed: "
              << result.error_message << '\n';
    RemoveTree(paths.test_root);
    return;
  }

  const fs::path payload_path = active_text_root / "2025-01.txt";
  const fs::path alias_config_path =
      config_root / "converter" / "alias_mapping.toml";
  const fs::path duration_config_path =
      config_root / "converter" / "duration_rules.toml";

  Expect(ReadTextFile(payload_path) ==
             CanonicalizeLegacyTextForAssertion(legacy_payload),
         "Imported payload TXT should be rewritten to canonical text.",
         failures);
  Expect(ReadTextFile(main_config_path) ==
             CanonicalizeLegacyTextForAssertion(legacy_main),
         "Applied main config should be rewritten to canonical text.",
         failures);
  Expect(ReadTextFile(alias_config_path) ==
             CanonicalizeLegacyTextForAssertion(legacy_alias),
         "Applied alias config should be rewritten to canonical text.",
         failures);
  Expect(ReadTextFile(duration_config_path) ==
             CanonicalizeLegacyTextForAssertion(legacy_duration),
         "Applied duration config should be rewritten to canonical text.",
         failures);
  Expect(result.text_root_updated && result.config_applied &&
             result.database_rebuilt,
         "Legacy canonicalized import should complete the full transaction.",
         failures);

  RemoveTree(paths.test_root);
}

auto TestTracerExchangeImportPreservesExtraMonthsAndRebuildsDatabase(
    int& failures) -> void {
  constexpr std::string_view kPassphrase = "phase3-tracer-exchange-passphrase";
  const RuntimeTestPaths paths =
      BuildTempTestPaths("tracer_core_tracer_exchange_transaction_import_test");
  const fs::path config_root = paths.test_root / "config";
  const fs::path active_config_root = paths.test_root / "config" / "converter";
  const fs::path main_config_path =
      active_config_root / "interval_processor_config.toml";
  const fs::path alias_config_path = active_config_root / "alias_mapping.toml";
  const fs::path duration_config_path =
      active_config_root / "duration_rules.toml";
  const fs::path active_text_root = paths.test_root / "input" / "full";
  const fs::path runtime_work_root = paths.test_root / "work";
  const fs::path package_path = paths.test_root / "package" / "exchange.ttpkg";
  const fs::path tracer_path = paths.test_root / "package" / "sample.tracer";

  const std::string original_main = "active-main = \"original\"\n";
  const std::string original_alias = "active-alias = \"original\"\n";
  const std::string original_duration = "active-duration = \"original\"\n";
  const std::string preserved_month =
      "y2024\nm01\n0101\n0600 legacy_project r keep\n";
  const std::string package_main = ReadRepoConverterConfig(
      "assets/tracer_core/config/converter/interval_processor_config.toml");
  const std::string package_alias = ReadRepoConverterConfig(
      "assets/tracer_core/config/converter/alias_mapping.toml");
  const std::string package_duration = ReadRepoConverterConfig(
      "assets/tracer_core/config/converter/duration_rules.toml");

  if (!PrepareRuntimeFixture(paths, config_root, failures)) {
    return;
  }
  if (!WriteFileWithParents(main_config_path, original_main) ||
      !WriteFileWithParents(alias_config_path, original_alias) ||
      !WriteFileWithParents(duration_config_path, original_duration) ||
      !WriteFileWithParents(active_text_root / "2024-01.txt", preserved_month)) {
    ++failures;
    std::cerr << "[FAIL] Failed to prepare active import fixture.\n";
    RemoveTree(paths.test_root);
    return;
  }

  const auto package_entries = BuildValidPackageEntries(
      BuildSamplePayloads(), package_main, package_alias, package_duration);
  const auto package_bytes = exchange_pkg::EncodePackageBytes(package_entries);

  std::error_code error;
  fs::create_directories(package_path.parent_path(), error);
  if (error || !WriteBytes(package_path, package_bytes)) {
    ++failures;
    std::cerr << "[FAIL] Failed to write tracer exchange package bytes.\n";
    RemoveTree(paths.test_root);
    return;
  }

  const auto encrypt_result =
      file_crypto::EncryptFile(package_path, tracer_path, kPassphrase);
  if (!encrypt_result.ok()) {
    ++failures;
    std::cerr << "[FAIL] Encrypt error: " << encrypt_result.error_code << " | "
              << encrypt_result.error_message << '\n';
    RemoveTree(paths.test_root);
    return;
  }

  auto runtime = BuildTracerExchangeRuntime(paths, main_config_path, failures);
  if (!runtime.has_value()) {
    return;
  }

  const tracer_core::core::dto::TracerExchangeImportRequest request{
      .input_tracer_path = tracer_path,
      .active_text_root_path = active_text_root,
      .active_converter_main_config_path = main_config_path,
      .runtime_work_root = runtime_work_root,
      .passphrase = std::string(kPassphrase),
  };

  const auto result =
      runtime->runtime_api->tracer_exchange().RunTracerExchangeImport(request);
  if (!result.ok) {
    ++failures;
    std::cerr
        << "[FAIL] RunTracerExchangeImport(transaction) failed unexpectedly: "
        << result.error_message << '\n';
    RemoveTree(paths.test_root);
    return;
  }

  Expect(result.config_applied,
         "RunTracerExchangeImport should report config application on success.",
         failures);
  Expect(result.text_root_updated,
         "RunTracerExchangeImport should report active text root update on success.",
         failures);
  Expect(result.database_rebuilt,
         "RunTracerExchangeImport should report database rebuild on success.",
         failures);
  Expect(result.replaced_month_count == BuildSamplePayloads().size(),
         "RunTracerExchangeImport should report replaced package month count.",
         failures);
  Expect(result.preserved_month_count == 1U,
         "RunTracerExchangeImport should preserve local months outside the package.",
         failures);
  Expect(result.rebuilt_month_count == BuildSamplePayloads().size() + 1U,
         "RunTracerExchangeImport should rebuild the preserved and replaced month set.",
         failures);
  Expect(!result.backup_retained_root.has_value(),
         "Successful transaction import should not retain backup_root.",
         failures);
  Expect(ReadTextFile(main_config_path) == package_main,
         "RunTracerExchangeImport should overwrite active main config.",
         failures);
  Expect(ReadTextFile(alias_config_path) == package_alias,
         "RunTracerExchangeImport should overwrite active alias config.",
         failures);
  Expect(ReadTextFile(duration_config_path) == package_duration,
         "RunTracerExchangeImport should overwrite active duration config.",
         failures);
  Expect(ReadTextFile(active_text_root / "2024-01.txt") == preserved_month,
         "Transaction import should preserve package-external local months.",
         failures);
  Expect(ReadTextFile(active_text_root / "2025-01.txt") ==
             BuildSamplePayloads().front().text,
         "Transaction import should replace package months in active text root.",
         failures);
  Expect(ReadTextFile(active_text_root / "2026-12.txt") ==
             BuildSamplePayloads().back().text,
         "Transaction import should write every package payload into active text root.",
         failures);
  Expect(fs::exists(paths.db_path),
         "Transaction import should rebuild the runtime database.",
         failures);

  RemoveTree(paths.test_root);
}

auto TestTracerExchangeImportApplyFailureRollsBackConfig(int& failures) -> void {
  constexpr std::string_view kPassphrase = "phase3-tracer-exchange-passphrase";
  const RuntimeTestPaths paths =
      BuildTempTestPaths("tracer_core_tracer_exchange_transaction_rollback_test");
  const fs::path config_root = paths.test_root / "config";
  const fs::path active_config_root = paths.test_root / "config" / "converter";
  const fs::path main_config_path =
      active_config_root / "interval_processor_config.toml";
  const fs::path alias_config_path = active_config_root / "alias_mapping.toml";
  const fs::path duration_config_path =
      active_config_root / "duration_rules.toml";
  const fs::path active_text_root = paths.test_root / "input" / "full";
  const fs::path runtime_work_root = paths.test_root / "work";
  const fs::path package_path = paths.test_root / "package" / "exchange.ttpkg";
  const fs::path tracer_path = paths.test_root / "package" / "rollback.tracer";

  const std::string original_main = "active-main = \"original\"\n";
  const std::string original_alias = "active-alias = \"original\"\n";
  const std::string original_duration = "active-duration = \"original\"\n";
  const std::string original_month = "y2024\nm01\n0101\n0600 keep r stable\n";
  const std::string package_main = ReadRepoConverterConfig(
      "assets/tracer_core/config/converter/interval_processor_config.toml");
  const std::string package_alias = ReadRepoConverterConfig(
      "assets/tracer_core/config/converter/alias_mapping.toml");
  const std::string package_duration = ReadRepoConverterConfig(
      "assets/tracer_core/config/converter/duration_rules.toml");

  if (!PrepareRuntimeFixture(paths, config_root, failures)) {
    return;
  }
  if (!WriteFileWithParents(main_config_path, original_main) ||
      !WriteFileWithParents(alias_config_path, original_alias) ||
      !WriteFileWithParents(duration_config_path, original_duration) ||
      !WriteFileWithParents(active_text_root / "2024-01.txt", original_month)) {
    ++failures;
    std::cerr
        << "[FAIL] Failed to prepare active transaction import files.\n";
    RemoveTree(paths.test_root);
    return;
  }
  if (!WriteEncryptedTracerFromEntries(
          package_path, tracer_path,
          BuildValidPackageEntries(BuildSamplePayloads(), package_main,
                                   package_alias, package_duration),
          kPassphrase, failures)) {
    RemoveTree(paths.test_root);
    return;
  }

  auto runtime = BuildTracerExchangeRuntime(paths, main_config_path, failures);
  if (!runtime.has_value()) {
    return;
  }

  if (!SetReadOnlyFlag(alias_config_path, true)) {
    ++failures;
    std::cerr << "[FAIL] Failed to mark alias_mapping.toml read-only for rollback test.\n";
    RemoveTree(paths.test_root);
    return;
  }

  const tracer_core::core::dto::TracerExchangeImportRequest request{
      .input_tracer_path = tracer_path,
      .active_text_root_path = active_text_root,
      .active_converter_main_config_path = main_config_path,
      .runtime_work_root = runtime_work_root,
      .passphrase = std::string(kPassphrase),
  };

  const auto result =
      runtime->runtime_api->tracer_exchange().RunTracerExchangeImport(request);
  static_cast<void>(SetReadOnlyFlag(alias_config_path, false));

  Expect(!result.ok,
         "RunTracerExchangeImport should fail when converter apply rollback is triggered.",
         failures);
  Expect(result.retained_failure_root.has_value(),
         "Failed transaction import should retain a failure root for debugging.",
         failures);
  Expect(NormalizeLf(ReadTextFile(main_config_path)) == original_main,
         "Rollback should restore the original main config after apply failure.",
         failures);
  Expect(NormalizeLf(ReadTextFile(alias_config_path)) == original_alias,
         "Rollback should preserve the original alias config after apply failure.",
         failures);
  Expect(NormalizeLf(ReadTextFile(duration_config_path)) == original_duration,
         "Rollback should preserve the original duration config after apply failure.",
         failures);
  Expect(ReadTextFile(active_text_root / "2024-01.txt") == original_month,
         "Rollback should preserve the original active text root content.",
         failures);
  Expect(!fs::exists(active_text_root / "2025-01.txt"),
         "Rollback should not leave newly imported month files in active text root.",
         failures);
  Expect(fs::exists(*result.retained_failure_root),
         "Failure root should exist on disk for debugging.",
         failures);

  RemoveTree(paths.test_root);
}

auto TestTracerExchangeImportRejectsInvalidConverterConfig(int& failures)
    -> void {
  constexpr std::string_view kPassphrase = "phase3-tracer-exchange-passphrase";
  const RuntimeTestPaths paths =
      BuildTempTestPaths("tracer_core_tracer_exchange_invalid_config_test");
  const fs::path config_root = paths.test_root / "config";
  const fs::path active_config_root = paths.test_root / "config" / "converter";
  const fs::path main_config_path =
      active_config_root / "interval_processor_config.toml";
  const fs::path alias_config_path = active_config_root / "alias_mapping.toml";
  const fs::path duration_config_path =
      active_config_root / "duration_rules.toml";
  const fs::path active_text_root = paths.test_root / "input" / "full";
  const fs::path runtime_work_root = paths.test_root / "work";
  const fs::path package_path = paths.test_root / "package" / "exchange.ttpkg";
  const fs::path tracer_path = paths.test_root / "package" / "broken.tracer";

  const std::string original_main = "active-main = \"original\"\n";
  const std::string original_alias = "active-alias = \"original\"\n";
  const std::string original_duration = "active-duration = \"original\"\n";

  if (!PrepareRuntimeFixture(paths, config_root, failures)) {
    return;
  }
  if (!WriteFileWithParents(main_config_path, original_main) ||
      !WriteFileWithParents(alias_config_path, original_alias) ||
      !WriteFileWithParents(duration_config_path, original_duration)) {
    ++failures;
    std::cerr << "[FAIL] Failed to prepare active converter config files.\n";
    RemoveTree(paths.test_root);
    return;
  }

  const auto package_entries = BuildValidPackageEntries(
      BuildSamplePayloads(), "invalid = [\n", "alias = \"broken\"\n",
      "duration = \"broken\"\n");
  const auto package_bytes = exchange_pkg::EncodePackageBytes(package_entries);

  std::error_code error;
  fs::create_directories(package_path.parent_path(), error);
  if (error || !WriteBytes(package_path, package_bytes)) {
    ++failures;
    std::cerr << "[FAIL] Failed to write tracer exchange package bytes.\n";
    RemoveTree(paths.test_root);
    return;
  }

  const auto encrypt_result =
      file_crypto::EncryptFile(package_path, tracer_path, kPassphrase);
  if (!encrypt_result.ok()) {
    ++failures;
    std::cerr << "[FAIL] Encrypt error: " << encrypt_result.error_code << " | "
              << encrypt_result.error_message << '\n';
    RemoveTree(paths.test_root);
    return;
  }

  auto runtime = BuildTracerExchangeRuntime(paths, main_config_path, failures);
  if (!runtime.has_value()) {
    return;
  }

  const tracer_core::core::dto::TracerExchangeImportRequest request{
      .input_tracer_path = tracer_path,
      .active_text_root_path = active_text_root,
      .active_converter_main_config_path = main_config_path,
      .runtime_work_root = runtime_work_root,
      .passphrase = std::string(kPassphrase),
  };

  const auto result =
      runtime->runtime_api->tracer_exchange().RunTracerExchangeImport(request);

  Expect(!result.ok,
         "RunTracerExchangeImport should reject invalid package converter config.",
         failures);
  Expect(Contains(ReadTextFile(main_config_path), "active-main = \"original\""),
         "Invalid package config should not overwrite active main config.",
         failures);
  Expect(
      Contains(ReadTextFile(alias_config_path), "active-alias = \"original\""),
      "Invalid package config should not overwrite active alias config.",
      failures);
  Expect(Contains(ReadTextFile(duration_config_path),
                  "active-duration = \"original\""),
         "Invalid package config should not overwrite active duration config.",
         failures);
  Expect(result.retained_failure_root.has_value(),
         "Failed import should retain a failure root for debugging.",
         failures);
  Expect(fs::exists(*result.retained_failure_root),
         "Retained failure root should exist on disk.", failures);
  Expect(!result.error_message.empty(),
         "Invalid package converter config should surface an error message.",
         failures);

  RemoveTree(paths.test_root);
}

}  // namespace

auto RunFileCryptoTracerExchangeImportTests(int& failures) -> void {
  TestTracerExchangeImportCanonicalizesLegacyText(failures);
  TestTracerExchangeImportPreservesExtraMonthsAndRebuildsDatabase(failures);
  TestTracerExchangeImportApplyFailureRollsBackConfig(failures);
  TestTracerExchangeImportRejectsInvalidConverterConfig(failures);
}

}  // namespace android_runtime_tests
