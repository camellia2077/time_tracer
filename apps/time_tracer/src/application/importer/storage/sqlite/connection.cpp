// importer/storage/sqlite/connection.cpp
#include "application/importer/storage/sqlite/connection.hpp"

#include <format>
#include <iostream>

#include "infrastructure/schema/day_schema.hpp"
#include "infrastructure/schema/time_records_schema.hpp"

Connection::Connection(const std::string& db_path) {       // MODIFIED
  if (sqlite3_open(db_path.c_str(), &db_) != SQLITE_OK) {  // MODIFIED
    std::cerr << "Error: Cannot open database: " << sqlite3_errmsg(db_)
              << std::endl;  // MODIFIED
    db_ = nullptr;           // MODIFIED
  } else {
    const std::string create_days_sql = std::format(
        "CREATE TABLE IF NOT EXISTS {0} ("
        "{1} TEXT PRIMARY KEY, "
        "{2} INTEGER, "
        "{3} INTEGER, "
        "{4} INTEGER, "
        "{5} INTEGER, "
        "{6} TEXT, "
        "{7} TEXT, "
        "{8} INTEGER, "
        "{9} INTEGER, "
        "{10} INTEGER, "
        "{11} INTEGER, "
        "{12} INTEGER, "
        "{13} INTEGER, "
        "{14} INTEGER, "
        "{15} INTEGER, "
        "{16} INTEGER, "
        "{17} INTEGER, "
        "{18} INTEGER, "
        "{19} INTEGER, "
        "{20} INTEGER, "
        "{21} INTEGER, "
        "{22} INTEGER);",
        schema::day::db::kTable, schema::day::db::kDate,
        schema::day::db::kYear,
        schema::day::db::kMonth, schema::day::db::kStatus,
        schema::day::db::kSleep, schema::day::db::kRemark,
        schema::day::db::kGetupTime, schema::day::db::kExercise,
        schema::day::db::kTotalExerciseTime, schema::day::db::kCardioTime,
        schema::day::db::kAnaerobicTime, schema::day::db::kGamingTime,
        schema::day::db::kGroomingTime, schema::day::db::kToiletTime,
        schema::day::db::kStudyTime, schema::day::db::kSleepNightTime,
        schema::day::db::kSleepDayTime, schema::day::db::kSleepTotalTime,
        schema::day::db::kRecreationTime,
        schema::day::db::kRecreationZhihuTime,
        schema::day::db::kRecreationBilibiliTime,
        schema::day::db::kRecreationDouyinTime);
    ExecuteSql(db_, create_days_sql, "Create days table");  // MODIFIED

    const std::string create_index_sql = std::format(
        "CREATE INDEX IF NOT EXISTS {0} ON {1} ({2}, {3});",
        schema::day::db::kIndexYearMonth, schema::day::db::kTable,
        schema::day::db::kYear, schema::day::db::kMonth);
    ExecuteSql(db_, create_index_sql,
               "Create index on days(year, month)");  // MODIFIED

    const std::string create_projects_sql = std::format(
        "CREATE TABLE IF NOT EXISTS {0} ("
        "{1} INTEGER PRIMARY KEY AUTOINCREMENT, "
        "{2} TEXT NOT NULL, "
        "{3} INTEGER, "
        "FOREIGN KEY ({3}) REFERENCES {0}({1}));",
        schema::projects::db::kTable, schema::projects::db::kId,
        schema::projects::db::kName, schema::projects::db::kParentId);
    ExecuteSql(db_, create_projects_sql,
               "Create projects table");  // MODIFIED

    const std::string create_records_sql = std::format(
        "CREATE TABLE IF NOT EXISTS {0} ("
        "{1} INTEGER PRIMARY KEY, "
        "{2} INTEGER, "
        "{3} INTEGER, "
        "{4} TEXT, "
        "{5} TEXT, "
        "{6} TEXT, "
        "{7} INTEGER, "
        "{8} INTEGER, "
        "{9} TEXT, "
        "FOREIGN KEY ({4}) REFERENCES {10}({11}), "
        "FOREIGN KEY ({7}) REFERENCES {12}({13}));",
        schema::time_records::db::kTable, schema::time_records::db::kLogicalId,
        schema::time_records::db::kStartTimestamp,
        schema::time_records::db::kEndTimestamp,
        schema::time_records::db::kDate, schema::time_records::db::kStart,
        schema::time_records::db::kEnd, schema::time_records::db::kProjectId,
        schema::time_records::db::kDuration,
        schema::time_records::db::kActivityRemark, schema::day::db::kTable,
        schema::day::db::kDate, schema::projects::db::kTable,
        schema::projects::db::kId);
    ExecuteSql(db_, create_records_sql,
               "Create time_records table");  // MODIFIED
  }
}

Connection::~Connection() {
  if (db_ != nullptr) {  // MODIFIED
    sqlite3_close(db_);  // MODIFIED
  }
}

auto Connection::GetDb() const -> sqlite3* {
  return db_;  // MODIFIED
}

auto Connection::BeginTransaction() -> bool {
  return ExecuteSql(db_, "BEGIN TRANSACTION;",
                    "Begin transaction");  // MODIFIED
}

auto Connection::CommitTransaction() -> bool {
  return ExecuteSql(db_, "COMMIT;", "Commit transaction");  // MODIFIED
}

auto Connection::RollbackTransaction() -> void {
  ExecuteSql(db_, "ROLLBACK;", "Rollback transaction");  // MODIFIED
}

// NOLINTBEGIN(bugprone-easily-swappable-parameters)
auto ExecuteSql(sqlite3* sqlite_db, const std::string& sql_query,
                const std::string& error_context) -> bool {
  char* err_msg = nullptr;
  if (sqlite3_exec(sqlite_db, sql_query.c_str(), nullptr, nullptr, &err_msg) !=
      SQLITE_OK) {
    std::cerr << "SQL Error (" << error_context << "): " << err_msg
              << std::endl;
    sqlite3_free(err_msg);
    return false;
  }
  return true;
}
// NOLINTEND(bugprone-easily-swappable-parameters)
