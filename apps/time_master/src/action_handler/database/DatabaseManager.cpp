// action_handler/database/DatabaseManager.cpp

#include "DatabaseManager.hpp"
#include <sqlite3.h> // SQLite 库头文件

DatabaseManager::DatabaseManager(const std::string& db_name)
    : db_name_(db_name), db_(nullptr) {}

DatabaseManager::~DatabaseManager() {
    close_database();
}

bool DatabaseManager::open_database_if_needed() {
    if (db_ != nullptr) {
        return true; // 连接已打开
    }

    if (!std::filesystem::exists(db_name_)) {
        std::cerr << RED_COLOR << "错误: 数据库文件 '" << db_name_ << "' 不存在。请先导入数据。" << RESET_COLOR << std::endl;
        return false;
    }

    if (sqlite3_open(db_name_.c_str(), &db_)) {
        std::cerr << RED_COLOR << "错误: 无法打开数据库 " << db_name_ << ": " << sqlite3_errmsg(db_) << RESET_COLOR << std::endl;
        sqlite3_close(db_); // 如果打开失败，也要确保关闭句柄
        db_ = nullptr;
        return false;
    }
    return true;
}

void DatabaseManager::close_database() {
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

sqlite3* DatabaseManager::get_db_connection() const {
    return db_;
}