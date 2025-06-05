// This file utilizes functions defined in common_utils.h, specifically time_str_to_seconds.
// Do not redefine functions that are already present in common_utils.h within this file.

#include "data_parser.h"
#include "common_utils.h" // 确保包含了 time_str_to_seconds 的定义
#include <fstream>
#include <iostream>
#include <regex>
#include <sqlite3.h>
#include <algorithm>

// --- FileDataParser 类的构造函数和析构函数实现 ---

FileDataParser::FileDataParser(const std::string& db_path) : db(nullptr), current_date_processed(false) {
    if (sqlite3_open(db_path.c_str(), &db)) {
        std::cerr << "错误：无法打开数据库: " << sqlite3_errmsg(db) << std::endl;
        db = nullptr;
    } else {
        _initialize_database();
        _prepopulate_parent_child();
    }
    // 初始化顶层父类别
    initial_top_level_parents = {
        {"study", "STUDY"},
        {"code", "CODE"}
    };
}

FileDataParser::~FileDataParser() {
    if (db) {
        // 在关闭前确保所有缓冲数据都已存储
        _store_previous_date_data();
        sqlite3_close(db);
    }
}


// --- FileDataParser 类的公有成员函数实现 ---

bool FileDataParser::is_db_open() const {
    return db != nullptr;
}

void FileDataParser::commit_all() {
    _store_previous_date_data();
}

bool FileDataParser::parse_file(const std::string& filename) {
    if (!db) return false;

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "错误: 无法打开文件 " << filename << std::endl;
        return false;
    }

    std::string line;
    int line_num = 0;
    current_file_name = filename;

    while (std::getline(file, line)) {
        line_num++;
        // 修剪行首和行尾的空白字符
        line.erase(0, line.find_first_not_of(" \t\n\r\f\v"));
        line.erase(line.find_last_not_of(" \t\n\r\f\v") + 1);

        if (line.empty()) continue;

        try {
            if (line.rfind("Date:", 0) == 0) {
                _store_previous_date_data(); // 处理上一天的数据
                _handle_date_line(line);
            } else if (line.rfind("Status:", 0) == 0) {
                _handle_status_line(line);
            } else if (line.rfind("Remark:", 0) == 0) {
                _handle_remark_line(line);
            } else if (line.rfind("Getup:", 0) == 0) {
                _handle_getup_line(line);
            } else if (line.find('~') != std::string::npos) {
                _handle_time_record_line(line, line_num);
            } else {
                // 可选：报告无法识别的行
            }
        } catch (const std::exception& e) {
            std::cerr << current_file_name << ":" << line_num << ": 处理行时出错: " << line << " - " << e.what() << std::endl;
        }
    }
    _store_previous_date_data(); // 处理文件中的最后一天数据
    file.close();
    return true;
}

// --- FileDataParser 类的私有成员函数实现 ---

void FileDataParser::_initialize_database() {
    const char* create_days_table_sql = R"(
        CREATE TABLE IF NOT EXISTS days (
            date TEXT PRIMARY KEY, status TEXT, remark TEXT, getup_time TEXT
        );
    )";
    execute_sql_parser(db, create_days_table_sql, "创建 days 表");

    const char* create_time_records_table_sql = R"(
        CREATE TABLE IF NOT EXISTS time_records (
            date TEXT, start TEXT, end TEXT, project_path TEXT, duration INTEGER,
            PRIMARY KEY (date, start),
            FOREIGN KEY (date) REFERENCES days(date)
        );
    )";
    execute_sql_parser(db, create_time_records_table_sql, "创建 time_records 表");

    const char* create_parent_child_table_sql = R"(
        CREATE TABLE IF NOT EXISTS parent_child (child TEXT PRIMARY KEY, parent TEXT);
    )";
    execute_sql_parser(db, create_parent_child_table_sql, "创建 parent_child 表");
}

void FileDataParser::_prepopulate_parent_child() {
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "INSERT OR IGNORE INTO parent_child (child, parent) VALUES (?, ?);";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return;

    for (const auto& pair : initial_top_level_parents) {
        sqlite3_bind_text(stmt, 1, pair.first.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, pair.second.c_str(), -1, SQLITE_STATIC);
        sqlite3_step(stmt);
        sqlite3_reset(stmt);
    }
    sqlite3_finalize(stmt);
}

void FileDataParser::_handle_date_line(const std::string& line) {
    if (line.length() > 5) {
        current_date = line.substr(5);
        current_date.erase(0, current_date.find_first_not_of(" \t"));
        current_date.erase(current_date.find_last_not_of(" \t") + 1);

        current_status = "False";
        current_remark = "";
        current_getup_time = "00:00";
        current_time_records_buffer.clear();
        current_date_processed = false;

        sqlite3_stmt* stmt;
        const char* sql = "INSERT OR IGNORE INTO days (date, status, remark, getup_time) VALUES (?, ?, ?, ?);";
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, current_date.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 2, current_status.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 3, current_remark.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 4, current_getup_time.c_str(), -1, SQLITE_STATIC);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
        }
    }
}

void FileDataParser::_handle_status_line(const std::string& line) {
    if (line.length() > 7) current_status = line.substr(7);
}

void FileDataParser::_handle_remark_line(const std::string& line) {
    if (line.length() > 7) current_remark = line.substr(7);
}

void FileDataParser::_handle_getup_line(const std::string& line) {
    if (line.length() > 6) current_getup_time = line.substr(6);
}

void FileDataParser::_handle_time_record_line(const std::string& line, int line_num) {
    // 注意：以下正则表达式用于匹配时间记录行，例如：03:31~11:53sleep_night
    // 根据业务逻辑，时间段与项目路径之间可能没有空格（例如 "11:53sleep_night" 是合法格式）
    // 因此，这里必须使用 \s*（允许0个或多个空白字符），不要改成 \s+  如果改为 \s+，将导致合法记录被错误忽略
    std::regex time_record_regex(R"((\d{2}:\d{2})~(\d{2}:\d{2})\s*(.+))");
    std::smatch matches;
    if (std::regex_match(line, matches, time_record_regex) && matches.size() == 4) {
        std::string start_time_str = matches[1].str();
        std::string end_time_str = matches[2].str();
        std::string project_path = matches[3].str();
        project_path.erase(project_path.find_last_not_of(" \t") + 1);

        int start_seconds = time_str_to_seconds(start_time_str);
        int end_seconds = time_str_to_seconds(end_time_str);
        int duration_seconds = (end_seconds < start_seconds) ? ((end_seconds + 24 * 3600) - start_seconds) : (end_seconds - start_seconds);

        current_time_records_buffer.push_back({current_date, start_time_str, end_time_str, project_path, duration_seconds});
        _process_project_path(project_path);
    }
}

void FileDataParser::_process_project_path(const std::string& project_path_orig) {
    std::string project_path = project_path_orig;
    std::replace(project_path.begin(), project_path.end(), ' ', '_');
    std::stringstream ss(project_path);
    std::string segment;
    std::vector<std::string> segments;
    while(std::getline(ss, segment, '_')) if (!segment.empty()) segments.push_back(segment);

    if (segments.empty()) return;

    sqlite3_stmt* stmt = nullptr;
    const char* sql = "INSERT OR IGNORE INTO parent_child (child, parent) VALUES (?, ?);";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return;

    std::string current_full_path = "";
    std::string parent_of_current_segment;
    for (size_t i = 0; i < segments.size(); ++i) {
        if (i == 0) {
            current_full_path = segments[i];
            auto it = initial_top_level_parents.find(current_full_path);
            parent_of_current_segment = (it != initial_top_level_parents.end()) ? it->second : current_full_path;
            if (it == initial_top_level_parents.end()) std::transform(parent_of_current_segment.begin(), parent_of_current_segment.end(), parent_of_current_segment.begin(), ::toupper);
        } else {
            parent_of_current_segment = current_full_path;
            current_full_path += "_" + segments[i];
        }
        sqlite3_bind_text(stmt, 1, current_full_path.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, parent_of_current_segment.c_str(), -1, SQLITE_STATIC);
        sqlite3_step(stmt);
        sqlite3_reset(stmt);
    }
    sqlite3_finalize(stmt);
}

void FileDataParser::_store_previous_date_data() {
    if (current_date.empty() || !db || current_date_processed) return;

    execute_sql_parser(db, "BEGIN TRANSACTION;", "开始事务");
    bool success = true;

    sqlite3_stmt* day_stmt;
    const char* update_day_sql = "UPDATE days SET status = ?, remark = ?, getup_time = ? WHERE date = ?;";
    if (success && sqlite3_prepare_v2(db, update_day_sql, -1, &day_stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(day_stmt, 1, current_status.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(day_stmt, 2, current_remark.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(day_stmt, 3, current_getup_time.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(day_stmt, 4, current_date.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(day_stmt) != SQLITE_DONE) success = false;
        sqlite3_finalize(day_stmt);
    } else success = false;

    sqlite3_stmt* record_stmt;
    const char* insert_record_sql = "INSERT OR REPLACE INTO time_records (date, start, end, project_path, duration) VALUES (?, ?, ?, ?, ?);";
    if (success && sqlite3_prepare_v2(db, insert_record_sql, -1, &record_stmt, nullptr) == SQLITE_OK) {
        for (const auto& record : current_time_records_buffer) {
            sqlite3_bind_text(record_stmt, 1, record.date.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(record_stmt, 2, record.start.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(record_stmt, 3, record.end.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(record_stmt, 4, record.project_path.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int(record_stmt, 5, record.duration_seconds);
            if (sqlite3_step(record_stmt) != SQLITE_DONE) {
                success = false;
                break;
            }
            sqlite3_reset(record_stmt);
        }
        sqlite3_finalize(record_stmt);
    } else success = false;

    execute_sql_parser(db, success ? "COMMIT;" : "ROLLBACK;", success ? "提交事务" : "回滚事务");

    current_time_records_buffer.clear();
    current_date_processed = true;
}

// 这是一个非成员函数，因为它在 .h 文件中是这样声明的
bool execute_sql_parser(sqlite3* db, const std::string& sql, const std::string& context_msg) {
    char* err_msg = nullptr;
    if (sqlite3_exec(db, sql.c_str(), 0, 0, &err_msg) != SQLITE_OK) {
        std::cerr << "SQL 错误 (" << context_msg << "): " << err_msg << std::endl;
        sqlite3_free(err_msg);
        return false;
    }
    return true;
}