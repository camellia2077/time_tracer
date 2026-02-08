// importer/storage/sqlite/statement.cpp
#include "application/importer/storage/sqlite/statement.hpp"

#include <format>
#include <stdexcept>

#include "infrastructure/schema/day_schema.hpp"
#include "infrastructure/schema/time_records_schema.hpp"

Statement::Statement(sqlite3* sqlite_db) : db_(sqlite_db) {
  PrepareStatements();
}

Statement::~Statement() {
  FinalizeStatements();
}

auto Statement::GetInsertDayStmt() const -> sqlite3_stmt* {
  return stmt_insert_day_;
}
auto Statement::GetInsertRecordStmt() const -> sqlite3_stmt* {
  return stmt_insert_record_;
}
auto Statement::GetInsertProjectStmt() const -> sqlite3_stmt* {
  return stmt_insert_project_;
}

void Statement::PrepareStatements() {
  const std::string insert_day_sql = std::format(
      "INSERT INTO {0} ("
      "{1}, {2}, {3}, {4}, {5}, {6}, {7}, "
      "{8}, {9}, {10}, {11}, "
      "{12}, {13}, {14}, "
      "{15}, {16}, {17}, "
      "{18}, {19}, {20}, {21}, "
      "{22}"
      ") "
      "VALUES ("
      "?, ?, ?, ?, ?, ?, ?, "
      "?, ?, ?, ?, "
      "?, ?, ?, "
      "?, ?, ?, "
      "?, ?, ?, ?, "
      "?"
      ");",
      schema::day::db::kTable, schema::day::db::kDate,
      schema::day::db::kYear,
      schema::day::db::kMonth, schema::day::db::kStatus,
      schema::day::db::kSleep, schema::day::db::kRemark,
      schema::day::db::kGetupTime, schema::day::db::kExercise,
      schema::day::db::kTotalExerciseTime, schema::day::db::kCardioTime,
      schema::day::db::kAnaerobicTime, schema::day::db::kGamingTime,
      schema::day::db::kGroomingTime, schema::day::db::kToiletTime,
      schema::day::db::kSleepNightTime, schema::day::db::kSleepDayTime,
      schema::day::db::kSleepTotalTime, schema::day::db::kRecreationTime,
      schema::day::db::kRecreationZhihuTime,
      schema::day::db::kRecreationBilibiliTime,
      schema::day::db::kRecreationDouyinTime, schema::day::db::kStudyTime);
  if (sqlite3_prepare_v2(db_, insert_day_sql.c_str(), -1, &stmt_insert_day_,
                         nullptr) !=
      SQLITE_OK) {
    throw std::runtime_error("Failed to prepare day insert statement.");
  }

  const std::string insert_record_sql = std::format(
      "INSERT OR REPLACE INTO {0} "
      "({1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}, {9}) "
      "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);",
      schema::time_records::db::kTable, schema::time_records::db::kLogicalId,
      schema::time_records::db::kStartTimestamp,
      schema::time_records::db::kEndTimestamp,
      schema::time_records::db::kDate, schema::time_records::db::kStart,
      schema::time_records::db::kEnd, schema::time_records::db::kProjectId,
      schema::time_records::db::kDuration,
      schema::time_records::db::kActivityRemark);
  if (sqlite3_prepare_v2(db_, insert_record_sql.c_str(), -1,
                         &stmt_insert_record_,
                         nullptr) != SQLITE_OK) {
    throw std::runtime_error("Failed to prepare time record insert statement.");
  }

  const std::string insert_project_sql = std::format(
      "INSERT INTO {0} ({1}, {2}) VALUES (?, ?);",
      schema::projects::db::kTable, schema::projects::db::kName,
      schema::projects::db::kParentId);
  if (sqlite3_prepare_v2(db_, insert_project_sql.c_str(), -1,
                         &stmt_insert_project_,
                         nullptr) != SQLITE_OK) {
    throw std::runtime_error("Failed to prepare project insert statement.");
  }
}

void Statement::FinalizeStatements() {
  if (stmt_insert_day_ != nullptr) {
    sqlite3_finalize(stmt_insert_day_);
  }
  if (stmt_insert_record_ != nullptr) {
    sqlite3_finalize(stmt_insert_record_);
  }
  if (stmt_insert_project_ != nullptr) {
    sqlite3_finalize(stmt_insert_project_);
  }
}
