// infrastructure/persistence/sqlite/db_manager.cpp

#include "infrastructure/persistence/sqlite/db_manager.hpp"

#include <sqlite3.h>

#include <utility>

#include "domain/ports/diagnostics.hpp"
#include "infrastructure/io/core/file_system_helper.hpp"

DBManager::DBManager(std::string db_name) : db_name_(std::move(db_name)) {}

DBManager::~DBManager() {
  CloseDatabase();
}

auto DBManager::OpenDatabaseIfNeeded() -> bool {
  if (db_ != nullptr) {
    return true;
  }

  // [New] 使用 FileSystemHelper 检查文件存在性
  if (!FileSystemHelper::Exists(db_name_)) {
    time_tracer::domain::ports::EmitError(
        std::string(time_tracer::common::colors::kRed) + "错误: 数据库文件 '" +
        db_name_ + "' 不存在。请先导入数据。" +
        std::string(time_tracer::common::colors::kReset));
    return false;
  }

  if (sqlite3_open(db_name_.c_str(), &db_) != 0) {
    time_tracer::domain::ports::EmitError(
        std::string(time_tracer::common::colors::kRed) +
        "错误: 无法打开数据库 " + db_name_ + ": " + sqlite3_errmsg(db_) +
        std::string(time_tracer::common::colors::kReset));
    sqlite3_close(db_);
    db_ = nullptr;
    return false;
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
