// infrastructure/tests/android_runtime/android_runtime_business_regression_support.cpp
#include <sqlite3.h>

#include <iostream>

#include "infrastructure/tests/android_runtime/android_runtime_business_regression_internal.hpp"

namespace android_runtime_tests::business_regression_internal {

auto ExpectNoDbArtifacts(const std::filesystem::path& db_path,
                         std::string_view context, int& failures) -> void {
  const std::filesystem::path wal_path =
      db_path.parent_path() / (db_path.filename().string() + "-wal");
  const std::filesystem::path shm_path =
      db_path.parent_path() / (db_path.filename().string() + "-shm");
  const std::filesystem::path journal_path =
      db_path.parent_path() / (db_path.filename().string() + "-journal");

  if (std::filesystem::exists(db_path) || std::filesystem::exists(wal_path) ||
      std::filesystem::exists(shm_path) ||
      std::filesystem::exists(journal_path)) {
    ++failures;
    std::cerr << "[FAIL] " << context
              << " should not leave SQLite artifacts after failure.\n";
  }
}

auto BuildValidSingleMonthTxt() -> std::string {
  return "y2026\n"
         "m02\n"
         "0201\n"
         "0700w\n"
         "0900rest\n"
         "1000meal\n";
}

auto BuildInvalidStructureSingleMonthTxt() -> std::string {
  return "y2026\n"
         "m02\n"
         "0201\n"
         "0700w\n"
         "0900wavebits\n"
         "1000meal\n";
}

auto BuildInvalidLogicSingleMonthTxt() -> std::string {
  return "y2026\n"
         "m02\n"
         "0201\n"
         "0700w\n"
         "0900rest\n"
         "1000meal\n"
         "\n"
         "0203\n"
         "0700w\n"
         "0900rest\n"
         "1000meal\n";
}

auto QueryCount(sqlite3* database, const std::string& sql)
    -> std::optional<long long> {
  sqlite3_stmt* statement = nullptr;
  if (sqlite3_prepare_v2(database, sql.c_str(), -1, &statement, nullptr) !=
          SQLITE_OK ||
      statement == nullptr) {
    return std::nullopt;
  }

  std::optional<long long> result;
  if (sqlite3_step(statement) == SQLITE_ROW) {
    result = sqlite3_column_int64(statement, 0);
  }
  sqlite3_finalize(statement);
  return result;
}

}  // namespace android_runtime_tests::business_regression_internal
