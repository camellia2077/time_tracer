#ifndef INFRASTRUCTURE_TESTS_EXCHANGE_EXCHANGE_TEST_COMMON_HPP_
#define INFRASTRUCTURE_TESTS_EXCHANGE_EXCHANGE_TEST_COMMON_HPP_

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "infrastructure/tests/android_runtime/android_runtime_test_common.hpp"

struct sqlite3;

namespace android_runtime_tests::exchange_tests_internal {

auto Expect(bool condition, const std::string& message, int& failures) -> void;
auto ReadTextFile(const std::filesystem::path& path) -> std::string;
auto ReadBytes(const std::filesystem::path& path) -> std::vector<std::uint8_t>;
auto WriteBytes(const std::filesystem::path& path,
                const std::vector<std::uint8_t>& bytes) -> bool;
auto CountFilesByExtension(const std::filesystem::path& root,
                           std::string_view extension) -> std::size_t;
auto QueryCount(sqlite3* database, const std::string& sql)
    -> std::optional<long long>;
auto ResolveRepoRootForInterop() -> std::filesystem::path;

}  // namespace android_runtime_tests::exchange_tests_internal

namespace android_runtime_tests {

auto RunTracerExchangeRuntimeTests(int& failures) -> void;
auto RunTracerExchangePackageTests(int& failures) -> void;

}  // namespace android_runtime_tests

#endif  // INFRASTRUCTURE_TESTS_EXCHANGE_EXCHANGE_TEST_COMMON_HPP_
