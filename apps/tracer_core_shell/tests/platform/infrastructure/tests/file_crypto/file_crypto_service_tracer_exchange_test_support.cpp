// infrastructure/tests/file_crypto/file_crypto_service_tracer_exchange_test_support.cpp
import tracer.core.infrastructure.exchange;

#include <algorithm>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#endif

#include "infrastructure/tests/file_crypto/file_crypto_service_tracer_exchange_test_support.hpp"

namespace android_runtime_tests::tracer_exchange_tests_internal {

using namespace file_crypto_tests_internal;

auto ToBytes(std::string_view text) -> std::vector<std::uint8_t> {
  return {text.begin(), text.end()};
}

auto BuildEntry(std::string_view relative_path, std::string_view text)
    -> exchange_pkg::TracerExchangePackageEntry {
  exchange_pkg::TracerExchangePackageEntry entry{};
  entry.relative_path = std::string(relative_path);
  entry.data = ToBytes(text);
  entry.entry_flags = exchange_pkg::kStandardEntryFlags;
  return entry;
}

auto BuildValidPackageEntries(const std::vector<PayloadFixture>& payloads,
                              const std::string& main_config,
                              const std::string& alias_index_config,
                              const std::string& duration_config)
    -> std::vector<exchange_pkg::TracerExchangePackageEntry> {
  return BuildValidPackageEntries(payloads, main_config, alias_index_config,
                                  duration_config,
                                  BuildDefaultAliasChildConfigs());
}

auto BuildValidPackageEntries(
    const std::vector<PayloadFixture>& payloads, const std::string& main_config,
    const std::string& alias_index_config, const std::string& duration_config,
    const std::vector<PayloadFixture>& alias_child_configs)
    -> std::vector<exchange_pkg::TracerExchangePackageEntry> {
  std::vector<PayloadFixture> sorted_alias_child_configs = alias_child_configs;
  std::sort(sorted_alias_child_configs.begin(), sorted_alias_child_configs.end(),
            [](const PayloadFixture& left, const PayloadFixture& right) {
              return left.relative_path < right.relative_path;
            });

  exchange_pkg::TracerExchangeManifest manifest{};
  manifest.producer_platform = "windows";
  manifest.producer_app = "time_tracer_cli";
  manifest.created_at_utc = "2026-03-18T12:34:56Z";
  manifest.source_root_name = "data";
  manifest.payload_files.reserve(payloads.size());
  for (const auto& payload : payloads) {
    manifest.payload_files.push_back(payload.relative_path);
  }
  manifest.converter_alias_mapping_files.reserve(
      sorted_alias_child_configs.size());
  for (const auto& alias_child : sorted_alias_child_configs) {
    manifest.converter_alias_mapping_files.push_back(alias_child.relative_path);
  }

  std::vector<exchange_pkg::TracerExchangePackageEntry> entries;
  entries.reserve(exchange_pkg::kRequiredPackagePaths.size() +
                  sorted_alias_child_configs.size() + payloads.size());
  entries.push_back(BuildEntry(exchange_pkg::kManifestPath,
                               exchange_pkg::BuildManifestText(manifest)));
  entries.push_back(BuildEntry(exchange_pkg::kConverterMainPath, main_config));
  entries.push_back(
      BuildEntry(exchange_pkg::kAliasMappingIndexPath, alias_index_config));
  entries.push_back(
      BuildEntry(exchange_pkg::kDurationRulesPath, duration_config));
  for (const auto& alias_child : sorted_alias_child_configs) {
    entries.push_back(BuildEntry(alias_child.relative_path, alias_child.text));
  }
  for (const auto& payload : payloads) {
    entries.push_back(BuildEntry(payload.relative_path, payload.text));
  }
  return entries;
}

auto FindEntry(const exchange_pkg::DecodedTracerExchangePackage& package,
               std::string_view path)
    -> const exchange_pkg::TracerExchangePackageEntry* {
  for (const auto& entry : package.entries) {
    if (entry.relative_path == path) {
      return &entry;
    }
  }
  return nullptr;
}

auto ReplaceFirst(std::string text, std::string_view from, std::string_view to)
    -> std::string {
  const std::size_t position = text.find(from);
  if (position == std::string::npos) {
    return text;
  }
  text.replace(position, from.size(), to);
  return text;
}

auto NormalizeLf(std::string text) -> std::string {
  text.erase(std::remove(text.begin(), text.end(), '\r'), text.end());
  return text;
}

auto StripUtf8Bom(std::string text) -> std::string {
  if (text.rfind("\xEF\xBB\xBF", 0) == 0) {
    text.erase(0, 3);
  }
  return text;
}

auto BuildLegacyText(std::string text) -> std::string {
  const std::string normalized = NormalizeLf(StripUtf8Bom(std::move(text)));
  std::string legacy;
  legacy.reserve(normalized.size() + 3U);
  legacy += "\xEF\xBB\xBF";
  for (const char value : normalized) {
    if (value == '\n') {
      legacy += "\r\n";
    } else {
      legacy.push_back(value);
    }
  }
  return legacy;
}

auto CanonicalizeLegacyTextForAssertion(std::string text) -> std::string {
  return NormalizeLf(StripUtf8Bom(std::move(text)));
}

auto ReadRepoConverterConfig(std::string_view relative_path) -> std::string {
  return ReadTextFile(ResolveRepoRootForInterop() / fs::path(relative_path));
}

auto ReadLegacyRepoConverterConfig(std::string_view relative_path)
    -> std::string {
  return BuildLegacyText(ReadRepoConverterConfig(relative_path));
}

auto BuildDefaultAliasChildConfigs() -> std::vector<PayloadFixture> {
  return {{
      .relative_path = "config/converter/aliases/default.toml",
      .text = "parent = \"study\"\n\n[aliases]\n\"study\" = \"math\"\n",
  }};
}

auto BuildRepoAliasChildConfigs() -> std::vector<PayloadFixture> {
  const std::vector<std::string> relative_paths = {
      "assets/tracer_core/config/converter/aliases/meal.toml",
      "assets/tracer_core/config/converter/aliases/recreation.toml",
      "assets/tracer_core/config/converter/aliases/routine.toml",
      "assets/tracer_core/config/converter/aliases/sleep.toml",
      "assets/tracer_core/config/converter/aliases/rest.toml",
      "assets/tracer_core/config/converter/aliases/exercise.toml",
      "assets/tracer_core/config/converter/aliases/study.toml",
  };

  std::vector<PayloadFixture> entries;
  entries.reserve(relative_paths.size());
  for (const auto& relative_path : relative_paths) {
    const fs::path relative_fs_path(relative_path);
    entries.push_back({
        .relative_path =
            (fs::path("config") / "converter" / "aliases" /
             relative_fs_path.filename())
                .generic_string(),
        .text = ReadRepoConverterConfig(relative_path),
    });
  }
  return entries;
}

auto BuildSamplePayloads() -> std::vector<PayloadFixture> {
  return {
      {.relative_path = "payload/2025/2025-01.txt",
       .text = "y2025\nm01\n0101\n0600w\n0630meal\n0700rest\n"},
      {.relative_path = "payload/2026/2026-12.txt",
       .text = "y2026\nm12\n1201\n0630w\n0700rest\n0730meal\n"},
  };
}

auto BuildValidExportPayloads() -> std::vector<PayloadFixture> {
  return {
      {.relative_path = "payload/2025/2025-01.txt",
       .text = "y2025\nm01\n0101\n0600w\n0630meal\n0700rest\n"},
      {.relative_path = "payload/2025/2025-02.txt",
       .text = "y2025\nm02\n0201\n0630w\n0700rest\n0730meal\n"},
  };
}

auto ResolveInputPayloadPath(std::string_view relative_package_path)
    -> fs::path {
  return fs::path(relative_package_path).lexically_relative("payload");
}

auto SeedExportInputRoot(const fs::path& input_root,
                         const std::vector<PayloadFixture>& payloads) -> bool {
  bool seeded = true;
  for (const auto& payload : payloads) {
    seeded = seeded &&
             WriteFileWithParents(
                 input_root / ResolveInputPayloadPath(payload.relative_path),
                 payload.text);
  }
  return seeded;
}

auto WriteRawBytesWithParents(const fs::path& path,
                              std::span<const std::uint8_t> bytes) -> bool {
  std::error_code error;
  fs::create_directories(path.parent_path(), error);
  if (error) {
    return false;
  }
  return WriteBytes(path,
                    std::vector<std::uint8_t>(bytes.begin(), bytes.end()));
}

auto WriteEncryptedTracerFromEntries(
    const fs::path& package_path, const fs::path& tracer_path,
    const std::vector<exchange_pkg::TracerExchangePackageEntry>&
        package_entries,
    std::string_view passphrase, int& failures) -> bool {
  const auto package_bytes = exchange_pkg::EncodePackageBytes(package_entries);
  const file_crypto::FileCryptoPathContext path_context{
      .input_root_path = package_path.parent_path(),
      .output_root_path = tracer_path.parent_path(),
      .current_input_path = package_path,
      .current_output_path = tracer_path,
  };
  const auto encrypt_result = file_crypto::EncryptBytesToFile(
      package_bytes, tracer_path, passphrase, path_context);
  if (!encrypt_result.ok()) {
    ++failures;
    std::cerr << "[FAIL] Encrypt error: " << encrypt_result.error_code << " | "
              << encrypt_result.error_message << '\n';
    return false;
  }

  return true;
}

auto DecodeTracerPackage(const fs::path& tracer_path,
                         const fs::path& decrypted_package_path,
                         std::string_view passphrase, int& failures)
    -> std::optional<exchange_pkg::DecodedTracerExchangePackage> {
  const file_crypto::FileCryptoPathContext path_context{
      .input_root_path = tracer_path.parent_path(),
      .output_root_path = decrypted_package_path.parent_path(),
      .current_input_path = tracer_path,
      .current_output_path = decrypted_package_path,
  };
  auto [decrypt_result, package_bytes] = file_crypto::DecryptFileToBytes(
      tracer_path, std::string(passphrase), path_context);
  if (!decrypt_result.ok()) {
    ++failures;
    std::cerr << "[FAIL] DecryptFileToBytes(tracer package) failed "
                 "unexpectedly: "
              << decrypt_result.error_code << " | "
              << decrypt_result.error_message << '\n';
    return std::nullopt;
  }

  try {
    return exchange_pkg::DecodePackageBytes(package_bytes);
  } catch (const std::exception& error) {
    ++failures;
    std::cerr << "[FAIL] DecodePackageBytes failed unexpectedly: "
              << error.what() << '\n';
    return std::nullopt;
  }
}

auto SetReadOnlyFlag(const fs::path& path, bool read_only) -> bool {
#if defined(_WIN32)
  const auto attributes = GetFileAttributesW(path.c_str());
  if (attributes == INVALID_FILE_ATTRIBUTES) {
    return false;
  }
  DWORD updated_attributes = attributes;
  if (read_only) {
    updated_attributes |= FILE_ATTRIBUTE_READONLY;
  } else {
    updated_attributes &= ~FILE_ATTRIBUTE_READONLY;
  }
  return SetFileAttributesW(path.c_str(), updated_attributes) != 0;
#else
  std::error_code error;
  const auto current_status = fs::status(path, error);
  if (error) {
    return false;
  }
  auto permissions = current_status.permissions();
  const auto options = fs::perm_options::replace;
  if (read_only) {
    permissions &= ~(fs::perms::owner_write | fs::perms::group_write |
                     fs::perms::others_write);
  } else {
    permissions |= fs::perms::owner_write;
  }
  fs::permissions(path, permissions, options, error);
  return !error;
#endif
}

auto PrepareRuntimeFixture(const RuntimeTestPaths& paths,
                           const fs::path& config_root, int& failures) -> bool {
  RemoveTree(paths.test_root);
  if (!PrepareAndroidConfigFixture(config_root)) {
    ++failures;
    std::cerr << "[FAIL] Failed to prepare runtime config fixture.\n";
    RemoveTree(paths.test_root);
    return false;
  }
  return true;
}

auto BuildTracerExchangeRuntime(const RuntimeTestPaths& paths,
                                const fs::path& main_config_path, int& failures)
    -> std::optional<infrastructure::bootstrap::AndroidRuntime> {
  try {
    return infrastructure::bootstrap::BuildAndroidRuntime(
        BuildRuntimeRequest(paths, main_config_path));
  } catch (const std::exception& error) {
    ++failures;
    std::cerr << "[FAIL] BuildAndroidRuntime threw unexpectedly: "
              << error.what() << '\n';
    RemoveTree(paths.test_root);
    return std::nullopt;
  }
}

}  // namespace android_runtime_tests::tracer_exchange_tests_internal
