// infrastructure/tests/data_query/data_query_refactor_test_internal.hpp
#ifndef INFRASTRUCTURE_TESTS_DATA_QUERY_REFACTOR_TEST_INTERNAL_HPP_
#define INFRASTRUCTURE_TESTS_DATA_QUERY_REFACTOR_TEST_INTERNAL_HPP_

#include <sqlite3.h>

#include <filesystem>
#include <memory>
#include <optional>
#include <string>

namespace android_runtime_tests::data_query_refactor_internal {

struct SqliteCloser {
  void operator()(sqlite3* database) const;
};

using ScopedSqlite = std::unique_ptr<sqlite3, SqliteCloser>;

[[nodiscard]] auto OpenInMemoryDatabase() -> ScopedSqlite;
[[nodiscard]] auto SeedDataQueryFixture(sqlite3* database) -> bool;
[[nodiscard]] auto ReadFileText(const std::filesystem::path& target_path)
    -> std::optional<std::string>;

auto Expect(bool condition, const std::string& message, int& failures) -> void;
auto ExpectNear(double lhs, double rhs, const std::string& message,
                int& failures) -> void;

auto RunDataQueryRefactorStatsTests(int& failures) -> void;
auto RunDataQueryRefactorPeriodTests(int& failures) -> void;
auto RunDataQueryRefactorTreeTests(int& failures) -> void;

}  // namespace android_runtime_tests::data_query_refactor_internal

#endif  // INFRASTRUCTURE_TESTS_DATA_QUERY_REFACTOR_TEST_INTERNAL_HPP_
