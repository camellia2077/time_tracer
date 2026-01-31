// infrastructure/persistence/sqlite/db_manager.cpp

#include "db_manager.hpp"

#include <sqlite3.h>

#include <utility>

#include "io/core/file_system_helper.hpp"

DBManager::DBManager(std::string db_name)
    : db_name_(std::move(db_name)), db_(nullptr) {}

DBManager::~DBManager() {
  close_database();
}

auto DBManager::open_database_if_needed() -> bool {
  if (db_ != nullptr) {
    return true;
  }

  // [New] 使用 FileSystemHelper 检查文件存在性
  if (!FileSystemHelper::exists(db_name_)) {
    std::cerr << RED_COLOR << "错误: 数据库文件 '" << db_name_
              << "' 不存在。请先导入数据。" << RESET_COLOR << std::endl;
    return false;
  }

  if (sqlite3_open(db_name_.c_str(), &db_) != 0) {
    std::cerr << RED_COLOR << "错误: 无法打开数据库 " << db_name_ << ": "
              << sqlite3_errmsg(db_) << RESET_COLOR << std::endl;
    sqlite3_close(db_);
    db_ = nullptr;
    return false;
  }
  return true;
}

void DBManager::close_database() {
  if (db_ != nullptr) {
    sqlite3_close(db_);
    db_ = nullptr;
  }
}

auto DBManager::get_db_connection() const -> sqlite3* {
  return db_;
}
