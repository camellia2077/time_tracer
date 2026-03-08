// infrastructure/tests/android_runtime/android_runtime_business_regression_internal.hpp
#ifndef INFRASTRUCTURE_TESTS_ANDROID_RUNTIME_ANDROID_RUNTIME_BUSINESS_REGRESSION_INTERNAL_HPP_
#define INFRASTRUCTURE_TESTS_ANDROID_RUNTIME_ANDROID_RUNTIME_BUSINESS_REGRESSION_INTERNAL_HPP_

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

struct sqlite3;

namespace android_runtime_tests::business_regression_internal {

auto ExpectNoDbArtifacts(const std::filesystem::path& db_path,
                         std::string_view context, int& failures) -> void;
auto BuildValidSingleMonthTxt() -> std::string;
auto BuildInvalidStructureSingleMonthTxt() -> std::string;
auto BuildInvalidLogicSingleMonthTxt() -> std::string;
auto QueryCount(sqlite3* database, const std::string& sql)
    -> std::optional<long long>;

auto RunBusinessRegressionHistoryTests(int& failures) -> void;
auto RunBusinessRegressionIngestGuardTests(int& failures) -> void;

}  // namespace android_runtime_tests::business_regression_internal

#endif  // INFRASTRUCTURE_TESTS_ANDROID_RUNTIME_ANDROID_RUNTIME_BUSINESS_REGRESSION_INTERNAL_HPP_
