// infrastructure/tests/data_query/data_query_refactor_test_support.cpp
#include <cmath>
#include <fstream>
#include <iostream>
#include <iterator>

#include "infrastructure/tests/android_runtime/android_runtime_test_common.hpp"
#include "infrastructure/tests/data_query/data_query_refactor_test_internal.hpp"

namespace android_runtime_tests::data_query_refactor_internal {
namespace {

constexpr double kDoubleTolerance = 1e-9;

}  // namespace

void SqliteCloser::operator()(sqlite3* database) const {
  if (database != nullptr) {
    sqlite3_close(database);
  }
}

auto OpenInMemoryDatabase() -> ScopedSqlite {
  sqlite3* database = nullptr;
  if (sqlite3_open(":memory:", &database) != SQLITE_OK || database == nullptr) {
    if (database != nullptr) {
      sqlite3_close(database);
    }
    return {nullptr};
  }
  return ScopedSqlite(database);
}

auto SeedDataQueryFixture(sqlite3* database) -> bool {
  constexpr std::string_view kCreateDaysTable =
      "CREATE TABLE days ("
      "  date TEXT PRIMARY KEY,"
      "  year INTEGER NOT NULL,"
      "  month INTEGER NOT NULL"
      ");";
  constexpr std::string_view kCreateTimeRecordsTable =
      "CREATE TABLE time_records ("
      "  date TEXT NOT NULL,"
      "  duration INTEGER NOT NULL,"
      "  project_path_snapshot TEXT,"
      "  activity_remark TEXT"
      ");";
  constexpr std::string_view kInsertDays =
      "INSERT INTO days(date, year, month) VALUES "
      "('2026-02-01', 2026, 2),"
      "('2026-02-02', 2026, 2),"
      "('2026-02-03', 2026, 2);";
  constexpr std::string_view kInsertRecords =
      "INSERT INTO time_records(date, duration, project_path_snapshot, "
      "activity_remark) VALUES "
      "('2026-02-01', 3600, 'study_cpp', ''),"
      "('2026-02-03', 1800, 'study_cpp', '');";

  return ExecuteSql(database, std::string(kCreateDaysTable)) &&
         ExecuteSql(database, std::string(kCreateTimeRecordsTable)) &&
         ExecuteSql(database, std::string(kInsertDays)) &&
         ExecuteSql(database, std::string(kInsertRecords));
}

auto ReadFileText(const std::filesystem::path& target_path)
    -> std::optional<std::string> {
  std::ifstream input(target_path, std::ios::binary);
  if (!input.is_open()) {
    return std::nullopt;
  }
  std::string content((std::istreambuf_iterator<char>(input)),
                      std::istreambuf_iterator<char>());
  return content;
}

auto Expect(bool condition, const std::string& message, int& failures) -> void {
  if (condition) {
    return;
  }
  ++failures;
  std::cerr << "[FAIL] " << message << '\n';
}

auto ExpectNear(double lhs, double rhs, const std::string& message,
                int& failures) -> void {
  Expect(std::abs(lhs - rhs) < kDoubleTolerance, message, failures);
}

}  // namespace android_runtime_tests::data_query_refactor_internal
