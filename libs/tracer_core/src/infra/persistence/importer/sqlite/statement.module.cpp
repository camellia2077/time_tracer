module;

#include <sqlite3.h>

#include <format>
#include <stdexcept>
#include <string>

#include "infra/schema/day_schema.hpp"
#include "infra/schema/sqlite_schema.hpp"

module tracer.core.infrastructure.persistence.write.importer.sqlite.statement;

namespace tracer::core::infrastructure::persistence::importer::sqlite {

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

auto Statement::PrepareStatements() -> void {
  const std::string kInsertDaySql = std::format(
      "INSERT INTO {0} ("
      "{1}, {2}, {3}, {4}, {5}, {6}"
      ") "
      "VALUES ("
      "?, ?, ?, ?, ?, ?"
      ") "
      "ON CONFLICT({1}) DO UPDATE SET "
      "{2}=excluded.{2}, "
      "{3}=excluded.{3}, "
      "{4}=excluded.{4}, "
      "{5}=excluded.{5}, "
      "{6}=excluded.{6};",
      schema::day::db::kTable, schema::day::db::kDate, schema::day::db::kYear,
      schema::day::db::kMonth, schema::day::db::kWakeAnchor,
      schema::day::db::kRemark, schema::day::db::kGetupTime);
  if (sqlite3_prepare_v2(db_, kInsertDaySql.c_str(), -1, &stmt_insert_day_,
                         nullptr) != SQLITE_OK) {
    throw std::runtime_error("Failed to prepare day insert statement.");
  }

  const std::string kInsertRecordSql = std::format(
      "INSERT INTO {0} "
      "({1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}, {9}, {10}) "
      "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?) "
      "ON CONFLICT({1}) DO UPDATE SET "
      "{2}=excluded.{2}, "
      "{3}=excluded.{3}, "
      "{4}=excluded.{4}, "
      "{5}=excluded.{5}, "
      "{6}=excluded.{6}, "
      "{7}=excluded.{7}, "
      "{8}=excluded.{8}, "
      "{9}=excluded.{9}, "
      "{10}=excluded.{10};",
      schema::time_records::db::kTable, schema::time_records::db::kLogicalId,
      schema::time_records::db::kStartTimestamp,
      schema::time_records::db::kEndTimestamp, schema::time_records::db::kDate,
      schema::time_records::db::kStart, schema::time_records::db::kEnd,
      schema::time_records::db::kProjectId, schema::time_records::db::kDuration,
      schema::time_records::db::kProjectPathSnapshot,
      schema::time_records::db::kActivityRemark);
  if (sqlite3_prepare_v2(db_, kInsertRecordSql.c_str(), -1,
                         &stmt_insert_record_, nullptr) != SQLITE_OK) {
    throw std::runtime_error("Failed to prepare time record insert statement.");
  }

  const std::string kInsertProjectSql = std::format(
      "INSERT INTO {0} ({1}, {2}, {3}, {4}) VALUES (?, ?, ?, ?);",
      schema::projects::db::kTable, schema::projects::db::kName,
      schema::projects::db::kParentId, schema::projects::db::kFullPath,
      schema::projects::db::kDepth);
  if (sqlite3_prepare_v2(db_, kInsertProjectSql.c_str(), -1,
                         &stmt_insert_project_, nullptr) != SQLITE_OK) {
    throw std::runtime_error("Failed to prepare project insert statement.");
  }
}

auto Statement::FinalizeStatements() -> void {
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

}  // namespace tracer::core::infrastructure::persistence::importer::sqlite
