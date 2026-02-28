// infrastructure/persistence/importer/sqlite/connection.cpp
#include "infrastructure/persistence/importer/sqlite/connection.hpp"

#include <format>
#include <optional>
#include <string>
#include <string_view>

#include "domain/ports/diagnostics.hpp"
#include "infrastructure/schema/day_schema.hpp"
#include "infrastructure/schema/sqlite_schema.hpp"

namespace infrastructure::persistence::importer::sqlite {
namespace {
auto QueryPragmaInt(sqlite3* sqlite_db, std::string_view sql)
    -> std::optional<int> {
  sqlite3_stmt* stmt = nullptr;
  const std::string kSql(sql);
  if (sqlite3_prepare_v2(sqlite_db, kSql.c_str(), -1, &stmt, nullptr) !=
      SQLITE_OK) {
    return std::nullopt;
  }

  std::optional<int> result;
  if (sqlite3_step(stmt) == SQLITE_ROW) {
    result = sqlite3_column_int(stmt, 0);
  }
  sqlite3_finalize(stmt);
  return result;
}

void LogForeignKeyStatus(sqlite3* sqlite_db) {
  const auto kStatus = QueryPragmaInt(sqlite_db, "PRAGMA foreign_keys;");
  if (!kStatus.has_value()) {
    tracer_core::domain::ports::EmitWarn(
        "[sqlite importer] failed to read PRAGMA foreign_keys status.");
    return;
  }
}
}  // namespace

Connection::Connection(const std::string& db_path) {
  if (sqlite3_open(db_path.c_str(), &db_) != SQLITE_OK) {
    db_ = nullptr;
  } else {
    if (!ExecuteSql(db_, "PRAGMA foreign_keys = ON;",
                    "Enable PRAGMA foreign_keys")) {
      tracer_core::domain::ports::EmitWarn(
          "[sqlite importer] failed to enable PRAGMA foreign_keys.");
    }
    LogForeignKeyStatus(db_);

    const std::string kCreateDaysSql = std::format(
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
        schema::day::db::kTable, schema::day::db::kDate, schema::day::db::kYear,
        schema::day::db::kMonth, schema::day::db::kStatus,
        schema::day::db::kSleep, schema::day::db::kRemark,
        schema::day::db::kGetupTime, schema::day::db::kExercise,
        schema::day::db::kTotalExerciseTime, schema::day::db::kCardioTime,
        schema::day::db::kAnaerobicTime, schema::day::db::kGamingTime,
        schema::day::db::kGroomingTime, schema::day::db::kToiletTime,
        schema::day::db::kStudyTime, schema::day::db::kSleepNightTime,
        schema::day::db::kSleepDayTime, schema::day::db::kSleepTotalTime,
        schema::day::db::kRecreationTime, schema::day::db::kRecreationZhihuTime,
        schema::day::db::kRecreationBilibiliTime,
        schema::day::db::kRecreationDouyinTime);
    ExecuteSql(db_, kCreateDaysSql, "Create days table");

    const std::string kCreateIndexSql =
        std::format("CREATE INDEX IF NOT EXISTS {0} ON {1} ({2}, {3});",
                    schema::day::db::kIndexYearMonth, schema::day::db::kTable,
                    schema::day::db::kYear, schema::day::db::kMonth);
    ExecuteSql(db_, kCreateIndexSql, "Create index on days(year, month)");

    const std::string kCreateProjectsSql = std::format(
        "CREATE TABLE IF NOT EXISTS {0} ("
        "{1} INTEGER PRIMARY KEY AUTOINCREMENT, "
        "{2} TEXT NOT NULL, "
        "{3} INTEGER, "
        "{4} TEXT NOT NULL DEFAULT '', "
        "{5} INTEGER NOT NULL DEFAULT 0, "
        "FOREIGN KEY ({3}) REFERENCES {0}({1}));",
        schema::projects::db::kTable, schema::projects::db::kId,
        schema::projects::db::kName, schema::projects::db::kParentId,
        schema::projects::db::kFullPath, schema::projects::db::kDepth);
    ExecuteSql(db_, kCreateProjectsSql, "Create projects table");

    const std::string kCreateProjectsFullPathUniqueSql = std::format(
        "CREATE UNIQUE INDEX IF NOT EXISTS idx_projects_full_path_unique ON "
        "{0} ({1}) WHERE {1} <> '';",
        schema::projects::db::kTable, schema::projects::db::kFullPath);
    ExecuteSql(db_, kCreateProjectsFullPathUniqueSql,
               "Create unique index on projects(full_path)");

    const std::string kCreateProjectsParentNameUniqueSql = std::format(
        "CREATE UNIQUE INDEX IF NOT EXISTS idx_projects_parent_name_unique ON "
        "{0} ({1}, {2});",
        schema::projects::db::kTable, schema::projects::db::kParentId,
        schema::projects::db::kName);
    ExecuteSql(db_, kCreateProjectsParentNameUniqueSql,
               "Create unique index on projects(parent_id, name)");

    const std::string kCreateRecordsSql = std::format(
        "CREATE TABLE IF NOT EXISTS {0} ("
        "{1} INTEGER PRIMARY KEY, "
        "{2} INTEGER NOT NULL CHECK ({2} >= 0), "
        "{3} INTEGER NOT NULL CHECK ({3} >= {2}), "
        "{4} TEXT NOT NULL, "
        "{5} TEXT NOT NULL, "
        "{6} TEXT NOT NULL, "
        "{7} INTEGER NOT NULL, "
        "{8} INTEGER NOT NULL CHECK ({8} >= 0), "
        "{9} TEXT NOT NULL DEFAULT '', "
        "{10} TEXT, "
        "FOREIGN KEY ({4}) REFERENCES {11}({12}), "
        "FOREIGN KEY ({7}) REFERENCES {13}({14}));",
        schema::time_records::db::kTable, schema::time_records::db::kLogicalId,
        schema::time_records::db::kStartTimestamp,
        schema::time_records::db::kEndTimestamp,
        schema::time_records::db::kDate, schema::time_records::db::kStart,
        schema::time_records::db::kEnd, schema::time_records::db::kProjectId,
        schema::time_records::db::kDuration,
        schema::time_records::db::kProjectPathSnapshot,
        schema::time_records::db::kActivityRemark, schema::day::db::kTable,
        schema::day::db::kDate, schema::projects::db::kTable,
        schema::projects::db::kId);
    ExecuteSql(db_, kCreateRecordsSql, "Create time_records table");

    const std::string kCreateRecordsDateProjectIndexSql = std::format(
        "CREATE INDEX IF NOT EXISTS idx_time_records_date_project ON {0} "
        "({1}, {2});",
        schema::time_records::db::kTable, schema::time_records::db::kDate,
        schema::time_records::db::kProjectId);
    ExecuteSql(db_, kCreateRecordsDateProjectIndexSql,
               "Create index on time_records(date, project_id)");

    const std::string kCreateRecordsDatePathSnapshotIndexSql = std::format(
        "CREATE INDEX IF NOT EXISTS idx_time_records_date_path_snapshot ON "
        "{0} ({1}, {2});",
        schema::time_records::db::kTable, schema::time_records::db::kDate,
        schema::time_records::db::kProjectPathSnapshot);
    ExecuteSql(db_, kCreateRecordsDatePathSnapshotIndexSql,
               "Create index on time_records(date, project_path_snapshot)");
  }
}

Connection::~Connection() {
  if (db_ != nullptr) {
    sqlite3_close(db_);
  }
}

auto Connection::GetDb() const -> sqlite3* {
  return db_;
}

auto Connection::BeginTransaction() -> bool {
  return ExecuteSql(db_, "BEGIN TRANSACTION;", "Begin transaction");
}

auto Connection::CommitTransaction() -> bool {
  return ExecuteSql(db_, "COMMIT;", "Commit transaction");
}

auto Connection::RollbackTransaction() -> void {
  ExecuteSql(db_, "ROLLBACK;", "Rollback transaction");
}

// NOLINTBEGIN(bugprone-easily-swappable-parameters)
auto ExecuteSql(sqlite3* sqlite_db, const std::string& sql_query,
                const std::string& error_context) -> bool {
  char* err_msg = nullptr;
  if (sqlite3_exec(sqlite_db, sql_query.c_str(), nullptr, nullptr, &err_msg) !=
      SQLITE_OK) {
    (void)error_context;
    sqlite3_free(err_msg);
    return false;
  }
  return true;
}
// NOLINTEND(bugprone-easily-swappable-parameters)

}  // namespace infrastructure::persistence::importer::sqlite
