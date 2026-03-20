// infrastructure/tests/file_crypto/file_crypto_service_tracer_exchange_export_tests.cpp
#include <string>
#include <string_view>

#include "infrastructure/tests/file_crypto/file_crypto_service_tracer_exchange_test_support.hpp"

namespace android_runtime_tests {
namespace {

using namespace file_crypto_tests_internal;
using namespace tracer_exchange_tests_internal;

auto TestTracerExchangeExportEndToEnd(int& failures) -> void {
  constexpr std::string_view kPassphrase = "phase3-tracer-exchange-passphrase";
  const RuntimeTestPaths paths =
      BuildTempTestPaths("tracer_core_tracer_exchange_export_test");
  const fs::path config_root = paths.test_root / "config";
  const fs::path main_config_path =
      config_root / "converter" / "interval_processor_config.toml";
  const fs::path input_root = paths.test_root / "input" / "data";
  const fs::path tracer_path = paths.test_root / "export" / "data.tracer";
  const fs::path decrypted_package_path =
      paths.test_root / "export" / "roundtrip.ttpkg";
  const auto payloads = BuildValidExportPayloads();

  if (!PrepareRuntimeFixture(paths, config_root, failures)) {
    return;
  }
  if (!SeedExportInputRoot(input_root, payloads)) {
    ++failures;
    std::cerr << "[FAIL] Failed to seed export input payload files.\n";
    RemoveTree(paths.test_root);
    return;
  }

  auto runtime = BuildTracerExchangeRuntime(paths, main_config_path, failures);
  if (!runtime.has_value()) {
    return;
  }

  const tracer_core::core::dto::TracerExchangeExportRequest request{
      .input_text_root_path = input_root,
      .requested_output_path = tracer_path,
      .active_converter_main_config_path = main_config_path,
      .date_check_mode = DateCheckMode::kNone,
      .passphrase = std::string(kPassphrase),
      .producer_platform = "android",
      .producer_app = "time_tracer_android",
      .security_level =
          tracer_core::core::dto::TracerExchangeSecurityLevel::kInteractive,
  };

  const auto result = runtime->core_api->RunTracerExchangeExport(request);
  if (!result.ok) {
    ++failures;
    std::cerr << "[FAIL] RunTracerExchangeExport failed unexpectedly: "
              << result.error_message << '\n';
    RemoveTree(paths.test_root);
    return;
  }

  Expect(result.resolved_output_tracer_path == fs::absolute(tracer_path),
         "RunTracerExchangeExport should honor the requested output path.",
         failures);
  Expect(result.source_root_name == "data",
         "RunTracerExchangeExport should preserve the input root name.",
         failures);
  Expect(result.payload_file_count == payloads.size(),
         "RunTracerExchangeExport should report payload_file_count.",
         failures);
  Expect(fs::exists(tracer_path),
         "RunTracerExchangeExport should materialize the .tracer artifact.",
         failures);

  const auto decrypt_result = file_crypto::DecryptFile(
      tracer_path, decrypted_package_path, std::string(kPassphrase));
  if (!decrypt_result.ok()) {
    ++failures;
    std::cerr << "[FAIL] DecryptFile(exported tracer) failed unexpectedly: "
              << decrypt_result.error_code << " | "
              << decrypt_result.error_message << '\n';
    RemoveTree(paths.test_root);
    return;
  }

  const auto package =
      exchange_pkg::DecodePackageBytes(ReadBytes(decrypted_package_path));
  Expect(package.manifest.source_root_name == "data",
         "Exported manifest should retain source_root_name.", failures);
  Expect(package.manifest.producer_platform == "android",
         "Exported manifest should stamp producer_platform from the host request.",
         failures);
  Expect(package.manifest.producer_app == "time_tracer_android",
         "Exported manifest should stamp producer_app from the host request.",
         failures);
  Expect(!package.manifest.created_at_utc.empty(),
         "Exported manifest should stamp created_at_utc.", failures);
  Expect(package.manifest.payload_files.size() == payloads.size(),
         "Exported manifest should list every payload file.", failures);
  Expect(FindEntry(package, exchange_pkg::kConverterMainPath) != nullptr,
         "Exported package should contain interval_processor_config.toml.",
         failures);
  Expect(FindEntry(package, exchange_pkg::kAliasMappingPath) != nullptr,
         "Exported package should contain alias_mapping.toml.", failures);
  Expect(FindEntry(package, exchange_pkg::kDurationRulesPath) != nullptr,
         "Exported package should contain duration_rules.toml.", failures);

  for (const auto& payload : payloads) {
    const auto* payload_entry = FindEntry(package, payload.relative_path);
    Expect(payload_entry != nullptr,
           "Exported package should retain every payload entry.", failures);
    if (payload_entry != nullptr) {
      const std::string payload_text(payload_entry->data.begin(),
                                     payload_entry->data.end());
      Expect(payload_text == payload.text,
             "Exported payload bytes should match source TXT content.",
             failures);
    }
  }

  RemoveTree(paths.test_root);
}

auto TestTracerExchangeInspectEndToEnd(int& failures) -> void {
  constexpr std::string_view kPassphrase = "phase3-tracer-exchange-passphrase";
  const RuntimeTestPaths paths =
      BuildTempTestPaths("tracer_core_tracer_exchange_inspect_test");
  const fs::path config_root = paths.test_root / "config";
  const fs::path main_config_path =
      config_root / "converter" / "interval_processor_config.toml";
  const fs::path package_path = paths.test_root / "package" / "inspect.ttpkg";
  const fs::path tracer_path = paths.test_root / "package" / "inspect.tracer";
  const auto payloads = BuildSamplePayloads();

  if (!PrepareRuntimeFixture(paths, config_root, failures)) {
    return;
  }
  if (!WriteEncryptedTracerFromEntries(
          package_path, tracer_path,
          BuildValidPackageEntries(payloads, "main = true\n", "alias = true\n",
                                   "duration = true\n"),
          kPassphrase, failures)) {
    RemoveTree(paths.test_root);
    return;
  }

  auto runtime = BuildTracerExchangeRuntime(paths, main_config_path, failures);
  if (!runtime.has_value()) {
    return;
  }

  const tracer_core::core::dto::TracerExchangeInspectRequest request{
      .input_tracer_path = tracer_path,
      .passphrase = std::string(kPassphrase),
  };

  const auto result = runtime->core_api->RunTracerExchangeInspect(request);
  if (!result.ok) {
    ++failures;
    std::cerr << "[FAIL] RunTracerExchangeInspect failed unexpectedly: "
              << result.error_message << '\n';
    RemoveTree(paths.test_root);
    return;
  }

  Expect(result.input_tracer_path == fs::absolute(tracer_path),
         "RunTracerExchangeInspect should report the inspected tracer path.",
         failures);
  Expect(result.package_type == "tracer_exchange",
         "RunTracerExchangeInspect should report package_type.", failures);
  Expect(result.package_version == 3,
         "RunTracerExchangeInspect should report package_version.", failures);
  Expect(result.producer_platform == "windows",
         "RunTracerExchangeInspect should report producer_platform.",
         failures);
  Expect(result.producer_app == "time_tracer_cli",
         "RunTracerExchangeInspect should report producer_app.", failures);
  Expect(result.created_at_utc == "2026-03-18T12:34:56Z",
         "RunTracerExchangeInspect should report created_at_utc.", failures);
  Expect(result.source_root_name == "data",
         "RunTracerExchangeInspect should report source_root_name.", failures);
  Expect(result.payload_file_count == payloads.size(),
         "RunTracerExchangeInspect should report payload_file_count.",
         failures);
  Expect(result.payload_entries.size() == payloads.size(),
         "RunTracerExchangeInspect should return a payload entry per TXT file.",
         failures);
  Expect(result.converter_entries[0].present,
         "RunTracerExchangeInspect should report interval_processor_config.toml.",
         failures);
  Expect(result.converter_entries[1].present,
         "RunTracerExchangeInspect should report alias_mapping.toml.",
         failures);
  Expect(result.converter_entries[2].present,
         "RunTracerExchangeInspect should report duration_rules.toml.",
         failures);

  RemoveTree(paths.test_root);
}

auto TestTracerExchangeExportCanonicalizesLegacyText(int& failures) -> void {
  constexpr std::string_view kPassphrase = "phase3-tracer-exchange-passphrase";
  const RuntimeTestPaths paths = BuildTempTestPaths(
      "tracer_core_tracer_exchange_export_canonical_text_test");
  const fs::path config_root = paths.test_root / "config";
  const fs::path main_config_path =
      config_root / "converter" / "interval_processor_config.toml";
  const fs::path alias_config_path =
      config_root / "converter" / "alias_mapping.toml";
  const fs::path duration_config_path =
      config_root / "converter" / "duration_rules.toml";
  const fs::path input_root = paths.test_root / "input" / "data";
  const fs::path tracer_path = paths.test_root / "export" / "legacy.tracer";
  const fs::path decrypted_package_path =
      paths.test_root / "export" / "legacy.ttpkg";
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
  if (!WriteRawBytesWithParents(input_root / "2025" / "2025-01.txt",
                                ToBytes(legacy_payload)) ||
      !WriteRawBytesWithParents(main_config_path, ToBytes(legacy_main)) ||
      !WriteRawBytesWithParents(alias_config_path, ToBytes(legacy_alias)) ||
      !WriteRawBytesWithParents(duration_config_path,
                                ToBytes(legacy_duration))) {
    ++failures;
    std::cerr << "[FAIL] Failed to seed legacy TXT/TOML bytes for export.\n";
    RemoveTree(paths.test_root);
    return;
  }

  auto runtime = BuildTracerExchangeRuntime(paths, main_config_path, failures);
  if (!runtime.has_value()) {
    return;
  }

  const auto result = runtime->core_api->RunTracerExchangeExport({
      .input_text_root_path = input_root,
      .requested_output_path = tracer_path,
      .active_converter_main_config_path = main_config_path,
      .date_check_mode = DateCheckMode::kNone,
      .passphrase = std::string(kPassphrase),
      .producer_platform = "windows",
      .producer_app = "time_tracer_cli",
      .security_level =
          tracer_core::core::dto::TracerExchangeSecurityLevel::kInteractive,
  });
  if (!result.ok) {
    ++failures;
    std::cerr << "[FAIL] Legacy text export should succeed: "
              << result.error_message << '\n';
    RemoveTree(paths.test_root);
    return;
  }

  const auto package_opt =
      DecodeTracerPackage(tracer_path, decrypted_package_path, kPassphrase,
                          failures);
  if (!package_opt.has_value()) {
    RemoveTree(paths.test_root);
    return;
  }
  const auto& package = *package_opt;

  const auto* payload_entry = FindEntry(package, "payload/2025/2025-01.txt");
  const auto* main_entry = FindEntry(package, exchange_pkg::kConverterMainPath);
  const auto* alias_entry = FindEntry(package, exchange_pkg::kAliasMappingPath);
  const auto* duration_entry =
      FindEntry(package, exchange_pkg::kDurationRulesPath);
  Expect(payload_entry != nullptr, "Canonical export should include payload entry.",
         failures);
  Expect(main_entry != nullptr, "Canonical export should include main TOML entry.",
         failures);
  Expect(alias_entry != nullptr,
         "Canonical export should include alias TOML entry.", failures);
  Expect(duration_entry != nullptr,
         "Canonical export should include duration TOML entry.", failures);

  if (payload_entry != nullptr) {
    Expect(std::string(payload_entry->data.begin(), payload_entry->data.end()) ==
               CanonicalizeLegacyTextForAssertion(legacy_payload),
           "Exported payload TXT bytes should be canonical UTF-8 text.",
           failures);
  }
  if (main_entry != nullptr) {
    Expect(std::string(main_entry->data.begin(), main_entry->data.end()) ==
               CanonicalizeLegacyTextForAssertion(legacy_main),
           "Exported main config bytes should be canonical UTF-8 text.",
           failures);
  }
  if (alias_entry != nullptr) {
    Expect(std::string(alias_entry->data.begin(), alias_entry->data.end()) ==
               CanonicalizeLegacyTextForAssertion(legacy_alias),
           "Exported alias config bytes should be canonical UTF-8 text.",
           failures);
  }
  if (duration_entry != nullptr) {
    Expect(std::string(duration_entry->data.begin(), duration_entry->data.end()) ==
               CanonicalizeLegacyTextForAssertion(legacy_duration),
           "Exported duration config bytes should be canonical UTF-8 text.",
           failures);
  }

  RemoveTree(paths.test_root);
}

auto TestTracerExchangeExportKeepsCanonicalTextStableAcrossHosts(int& failures)
    -> void {
  constexpr std::string_view kPassphrase = "phase3-tracer-exchange-passphrase";
  const RuntimeTestPaths paths = BuildTempTestPaths(
      "tracer_core_tracer_exchange_cross_host_canonical_text_test");
  const fs::path config_root = paths.test_root / "config";
  const fs::path main_config_path =
      config_root / "converter" / "interval_processor_config.toml";
  const fs::path input_root = paths.test_root / "input" / "data";
  const fs::path android_tracer = paths.test_root / "export" / "android.tracer";
  const fs::path windows_tracer = paths.test_root / "export" / "windows.tracer";
  const fs::path android_package = paths.test_root / "export" / "android.ttpkg";
  const fs::path windows_package = paths.test_root / "export" / "windows.ttpkg";
  const std::string legacy_payload =
      "\xEF\xBB\xBFy2025\r\nm01\r\n0101\r\n0600 study\r\n";

  if (!PrepareRuntimeFixture(paths, config_root, failures)) {
    return;
  }
  if (!WriteRawBytesWithParents(input_root / "2025" / "2025-01.txt",
                                ToBytes(legacy_payload))) {
    ++failures;
    std::cerr
        << "[FAIL] Failed to seed legacy TXT payload for cross-host export.\n";
    RemoveTree(paths.test_root);
    return;
  }

  auto runtime = BuildTracerExchangeRuntime(paths, main_config_path, failures);
  if (!runtime.has_value()) {
    return;
  }

  const auto android_result = runtime->core_api->RunTracerExchangeExport({
      .input_text_root_path = input_root,
      .requested_output_path = android_tracer,
      .active_converter_main_config_path = main_config_path,
      .date_check_mode = DateCheckMode::kNone,
      .passphrase = std::string(kPassphrase),
      .producer_platform = "android",
      .producer_app = "time_tracer_android",
      .security_level =
          tracer_core::core::dto::TracerExchangeSecurityLevel::kInteractive,
  });
  const auto windows_result = runtime->core_api->RunTracerExchangeExport({
      .input_text_root_path = input_root,
      .requested_output_path = windows_tracer,
      .active_converter_main_config_path = main_config_path,
      .date_check_mode = DateCheckMode::kNone,
      .passphrase = std::string(kPassphrase),
      .producer_platform = "windows",
      .producer_app = "time_tracer_cli",
      .security_level =
          tracer_core::core::dto::TracerExchangeSecurityLevel::kInteractive,
  });
  if (!android_result.ok || !windows_result.ok) {
    ++failures;
    std::cerr << "[FAIL] Cross-host exports should both succeed.\n";
    RemoveTree(paths.test_root);
    return;
  }

  const auto android_package_opt =
      DecodeTracerPackage(android_tracer, android_package, kPassphrase,
                          failures);
  const auto windows_package_opt =
      DecodeTracerPackage(windows_tracer, windows_package, kPassphrase,
                          failures);
  if (!android_package_opt.has_value() || !windows_package_opt.has_value()) {
    RemoveTree(paths.test_root);
    return;
  }

  for (const std::string_view path :
       {std::string_view("payload/2025/2025-01.txt"),
        exchange_pkg::kConverterMainPath, exchange_pkg::kAliasMappingPath,
        exchange_pkg::kDurationRulesPath}) {
    const auto* android_entry = FindEntry(*android_package_opt, path);
    const auto* windows_entry = FindEntry(*windows_package_opt, path);
    Expect(android_entry != nullptr && windows_entry != nullptr,
           "Cross-host export should contain every canonical text entry.",
           failures);
    if (android_entry != nullptr && windows_entry != nullptr) {
      Expect(android_entry->data == windows_entry->data,
             "Cross-host export should keep canonical text entry bytes stable.",
             failures);
    }
  }

  RemoveTree(paths.test_root);
}

}  // namespace

auto RunFileCryptoTracerExchangeExportTests(int& failures) -> void {
  TestTracerExchangeExportEndToEnd(failures);
  TestTracerExchangeInspectEndToEnd(failures);
  TestTracerExchangeExportCanonicalizesLegacyText(failures);
  TestTracerExchangeExportKeepsCanonicalTextStableAcrossHosts(failures);
}

}  // namespace android_runtime_tests
