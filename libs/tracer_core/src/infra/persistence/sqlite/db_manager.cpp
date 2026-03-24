// infra/persistence/sqlite/db_manager.cpp
#include "infra/persistence/sqlite/db_manager.hpp"

#include <sqlite3.h>

#include <filesystem>
#include <optional>
#include <utility>

import tracer.core.domain.ports.diagnostics;

namespace modports = tracer::core::domain::ports;

namespace {
auto QueryForeignKeysPragma(sqlite3* db_conn) -> std::optional<int> {
  sqlite3_stmt* stmt = nullptr;
  if (sqlite3_prepare_v2(db_conn, "PRAGMA foreign_keys;", -1, &stmt, nullptr) !=
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
}  // namespace

DBManager::DBManager(std::string db_name) : db_name_(std::move(db_name)) {}

DBManager::~DBManager() {
  CloseDatabase();
}

auto DBManager::OpenDatabaseIfNeeded() -> bool {
  if (db_ != nullptr) {
    return true;
  }

  if (!std::filesystem::exists(db_name_)) {
    modports::EmitError("错误: 数据库文件 '" + db_name_ +
                        "' 不存在。请先导入数据。");
    return false;
  }

  if (sqlite3_open(db_name_.c_str(), &db_) != 0) {
    modports::EmitError("错误: 无法打开数据库 " + db_name_ + ": " +
                        sqlite3_errmsg(db_));
    sqlite3_close(db_);
    db_ = nullptr;
    return false;
  }

  char* err_msg = nullptr;
  if (sqlite3_exec(db_, "PRAGMA foreign_keys = ON;", nullptr, nullptr,
                   &err_msg) != SQLITE_OK) {
    const std::string kError =
        (err_msg != nullptr) ? err_msg : "unknown sqlite error";
    sqlite3_free(err_msg);
    modports::EmitWarn("警告: 无法启用 PRAGMA foreign_keys: " + kError);
  }

  const auto kFkStatus = QueryForeignKeysPragma(db_);
  if (!kFkStatus.has_value()) {
    modports::EmitWarn("警告: 无法读取 PRAGMA foreign_keys 状态。");
  }

  return true;
}

void DBManager::CloseDatabase() {
  if (db_ != nullptr) {
    sqlite3_close(db_);
    db_ = nullptr;
  }
}

auto DBManager::GetDbConnection() const -> sqlite3* {
  return db_;
}
