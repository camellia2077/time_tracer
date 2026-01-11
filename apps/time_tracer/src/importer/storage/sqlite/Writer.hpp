// importer/storage/sqlite/Writer.hpp
#ifndef WRITER_HPP
#define WRITER_HPP

#include <vector>
#include <string>
#include <sqlite3.h>
#include <map>
#include "importer/model/TimeSheetData.hpp"

class Writer {
public:
    // --- [核心修改] 更新构造函数签名 ---
    explicit Writer(sqlite3* db, 
                          sqlite3_stmt* stmt_day, 
                          sqlite3_stmt* stmt_record, 
                          sqlite3_stmt* stmt_select_project, 
                          sqlite3_stmt* stmt_insert_project);

    void insert_days(const std::vector<DayData>& days);
    void insert_records(const std::vector<TimeRecordInternal>& records);
    // --- [核心修改] 移除 insert_parent_child_pairs ---

private:
    sqlite3* db_; // MODIFIED
    sqlite3_stmt* stmt_insert_day_; // MODIFIED
    sqlite3_stmt* stmt_insert_record_; // MODIFIED
    // --- [核心修改] 新增 projects 表相关的语句句柄 ---
    sqlite3_stmt* stmt_select_project_id_; // MODIFIED
    sqlite3_stmt* stmt_insert_project_; // MODIFIED

    // --- [核心修改] 新增用于缓存和处理项目ID的成员 ---
    std::map<std::string, long long> project_path_cache_;
    long long get_or_create_project_id(const std::string& project_path);
};

#endif // WRITER_HPP