// infrastructure/tests/file_crypto/file_crypto_service_tracer_exchange_test_support.hpp
#ifndef INFRASTRUCTURE_TESTS_FILE_CRYPTO_SERVICE_TRACER_EXCHANGE_TEST_SUPPORT_HPP_
#define INFRASTRUCTURE_TESTS_FILE_CRYPTO_SERVICE_TRACER_EXCHANGE_TEST_SUPPORT_HPP_

import tracer.core.infrastructure.exchange;

#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "host/bootstrap/android_runtime_factory.hpp"
#include "infrastructure/tests/file_crypto/file_crypto_service_test_internal.hpp"

namespace android_runtime_tests::tracer_exchange_tests_internal {

namespace fs = std::filesystem;
namespace exchange_pkg = tracer::core::infrastructure::crypto::exchange;
namespace file_crypto = tracer_core::infrastructure::crypto;

struct PayloadFixture {
  std::string relative_path;
  std::string text;
};

auto ToBytes(std::string_view text) -> std::vector<std::uint8_t>;
auto BuildEntry(std::string_view relative_path, std::string_view text)
    -> exchange_pkg::TracerExchangePackageEntry;
auto BuildValidPackageEntries(const std::vector<PayloadFixture>& payloads,
                              const std::string& main_config,
                              const std::string& alias_index_config,
                              const std::string& duration_config)
    -> std::vector<exchange_pkg::TracerExchangePackageEntry>;
auto BuildValidPackageEntries(
    const std::vector<PayloadFixture>& payloads, const std::string& main_config,
    const std::string& alias_index_config, const std::string& duration_config,
    const std::vector<PayloadFixture>& alias_child_configs)
    -> std::vector<exchange_pkg::TracerExchangePackageEntry>;
auto FindEntry(const exchange_pkg::DecodedTracerExchangePackage& package,
               std::string_view path)
    -> const exchange_pkg::TracerExchangePackageEntry*;
auto ReplaceFirst(std::string text, std::string_view from, std::string_view to)
    -> std::string;
auto NormalizeLf(std::string text) -> std::string;
auto StripUtf8Bom(std::string text) -> std::string;
auto BuildLegacyText(std::string text) -> std::string;
auto CanonicalizeLegacyTextForAssertion(std::string text) -> std::string;
auto ReadRepoConverterConfig(std::string_view relative_path) -> std::string;
auto ReadLegacyRepoConverterConfig(std::string_view relative_path)
    -> std::string;
auto BuildDefaultAliasChildConfigs() -> std::vector<PayloadFixture>;
auto BuildRepoAliasChildConfigs() -> std::vector<PayloadFixture>;
auto BuildSamplePayloads() -> std::vector<PayloadFixture>;
auto BuildValidExportPayloads() -> std::vector<PayloadFixture>;
auto ResolveInputPayloadPath(std::string_view relative_package_path)
    -> fs::path;
auto SeedExportInputRoot(const fs::path& input_root,
                         const std::vector<PayloadFixture>& payloads) -> bool;
auto WriteRawBytesWithParents(const fs::path& path,
                              std::span<const std::uint8_t> bytes) -> bool;
auto WriteEncryptedTracerFromEntries(
    const fs::path& package_path, const fs::path& tracer_path,
    const std::vector<exchange_pkg::TracerExchangePackageEntry>&
        package_entries,
    std::string_view passphrase, int& failures) -> bool;
auto DecodeTracerPackage(const fs::path& tracer_path,
                         const fs::path& decrypted_package_path,
                         std::string_view passphrase, int& failures)
    -> std::optional<exchange_pkg::DecodedTracerExchangePackage>;
auto SetReadOnlyFlag(const fs::path& path, bool read_only) -> bool;
auto PrepareRuntimeFixture(const RuntimeTestPaths& paths,
                           const fs::path& config_root, int& failures) -> bool;
auto BuildTracerExchangeRuntime(const RuntimeTestPaths& paths,
                                const fs::path& main_config_path, int& failures)
    -> std::optional<infrastructure::bootstrap::AndroidRuntime>;

}  // namespace android_runtime_tests::tracer_exchange_tests_internal

namespace android_runtime_tests {

auto RunFileCryptoTracerExchangePackageTests(int& failures) -> void;
auto RunFileCryptoTracerExchangeExportTests(int& failures) -> void;
auto RunFileCryptoTracerExchangeImportTests(int& failures) -> void;

}  // namespace android_runtime_tests

#endif  // INFRASTRUCTURE_TESTS_FILE_CRYPTO_SERVICE_TRACER_EXCHANGE_TEST_SUPPORT_HPP_
