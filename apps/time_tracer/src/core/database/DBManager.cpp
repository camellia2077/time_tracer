// core/database/DBManager.cpp

#include "DBManager.hpp"
#include "file_handler/FileSystemHelper.hpp" // [New]
#include <sqlite3.h> 

DBManager::DBManager(const std::string& db_name)
    : db_name_(db_name), db_(nullptr) {}

DBManager::~DBManager() {
    close_database();
}

bool DBManager::open_database_if_needed() {
    if (db_ != nullptr) {
        return true; 
    }

    // [New] 使用 FileSystemHelper 检查文件存在性
    if (!FileSystemHelper::exists(db_name_)) {
        std::cerr << RED_COLOR << "错误: 数据库文件 '" << db_name_ << "' 不存在。请先导入数据。" << RESET_COLOR << std::endl;
        return false;
    }

    if (sqlite3_open(db_name_.c_str(), &db_)) {
        std::cerr << RED_COLOR << "错误: 无法打开数据库 " << db_name_ << ": " << sqlite3_errmsg(db_) << RESET_COLOR << std::endl;
        sqlite3_close(db_); 
        db_ = nullptr;
        return false;
    }
    return true;
}

void DBManager::close_database() {
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

sqlite3* DBManager::get_db_connection() const {
    return db_;
}