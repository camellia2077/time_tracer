#include "database_inserter.h"
#include <iostream>
#include <stdexcept>
#include <vector>

// --- DatabaseInserter 构造函数与析构函数 ---

DatabaseInserter::DatabaseInserter(const std::string& db_path) 
    : db(nullptr), stmt_insert_day(nullptr), stmt_insert_record(nullptr), stmt_insert_parent_child(nullptr) {
    // 尝试打开数据库文件
    if (sqlite3_open(db_path.c_str(), &db)) {
        std::cerr << "Error: Cannot open database: " << sqlite3_errmsg(db) << std::endl;
        db = nullptr; // 打开失败，将指针设为null
    } else {
        // 打开成功后，初始化表结构并准备SQL语句
        _initialize_database();
        _prepare_statements();
    }
}

DatabaseInserter::~DatabaseInserter() {
    if (db) {
        // 在关闭数据库之前，确保所有资源都被正确释放
        _finalize_statements();
        sqlite3_close(db);
    }
}

// --- 公共成员函数 ---

// 检查数据库连接是否有效
bool DatabaseInserter::is_db_open() const {
    return db != nullptr;
}

// 将解析器中的数据导入数据库的核心函数
void DatabaseInserter::import_data(const DataFileParser& parser) {
    if (!db) return; // 如果数据库未打开，则直接返回

    // --- 性能优化：在事务开始前，临时禁用外键约束检查 ---
    // 这可以显著提高批量导入的速度，因为数据库无需在每次插入时都进行关联检查。
    execute_sql_Inserter(db, "PRAGMA foreign_keys = OFF;", "Disable foreign keys for import");
    
    // --- 性能优化：开启一个事务 ---
    // 将所有INSERT操作包裹在一个事务中，可以把成千上万次独立的磁盘写入操作合并为一次，
    // 这是批量插入时最重要的性能优化手段。
    execute_sql_Inserter(db, "BEGIN TRANSACTION;", "Begin import transaction");

    // 1. 遍历并插入 'days' 表的数据
    for (const auto& day_data : parser.days) {
        // 使用 sqlite3_bind_* 函数安全地将数据绑定到预编译语句的占位符(?)上
        sqlite3_bind_text(stmt_insert_day, 1, day_data.date.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt_insert_day, 2, day_data.status.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt_insert_day, 3, day_data.sleep.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt_insert_day, 4, day_data.remark.c_str(), -1, SQLITE_TRANSIENT);
        
        // 特殊处理可能为空的 'getup_time'
        if (day_data.getup_time == "Null" || day_data.getup_time.empty()) {
            sqlite3_bind_null(stmt_insert_day, 5); // 如果为空，则在数据库中存为NULL
        } else {
            sqlite3_bind_text(stmt_insert_day, 5, day_data.getup_time.c_str(), -1, SQLITE_TRANSIENT);
        }

        // 执行预编译好的插入语句
        if (sqlite3_step(stmt_insert_day) != SQLITE_DONE) {
            std::cerr << "Error inserting day row: " << sqlite3_errmsg(db) << std::endl;
        }
        sqlite3_reset(stmt_insert_day); // 重置语句，以便在下一次循环中复用
    }

    // 2. 遍历并插入 'time_records' 表的数据
    for (const auto& record_data : parser.records) {
        sqlite3_bind_text(stmt_insert_record, 1, record_data.date.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt_insert_record, 2, record_data.start.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt_insert_record, 3, record_data.end.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt_insert_record, 4, record_data.project_path.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt_insert_record, 5, record_data.duration_seconds);

        if (sqlite3_step(stmt_insert_record) != SQLITE_DONE) {
            std::cerr << "Error inserting record row: " << sqlite3_errmsg(db) << std::endl;
        }
        sqlite3_reset(stmt_insert_record);
    }

    // 3. 遍历并插入 'parent_child' 表的数据
    for (const auto& pair : parser.parent_child_pairs) {
        sqlite3_bind_text(stmt_insert_parent_child, 1, pair.first.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt_insert_parent_child, 2, pair.second.c_str(), -1, SQLITE_TRANSIENT);

        if (sqlite3_step(stmt_insert_parent_child) != SQLITE_DONE) {
            std::cerr << "Error inserting parent-child row: " << sqlite3_errmsg(db) << std::endl;
        }
        sqlite3_reset(stmt_insert_parent_child);
    }

    // --- 提交事务 ---
    // 所有操作成功完成后，提交事务，将所有更改一次性永久写入数据库文件。
    execute_sql_Inserter(db, "COMMIT;", "Commit import transaction");
    // 注意：在专业应用中，通常在此之后会使用 "PRAGMA foreign_keys = ON;" 来恢复约束检查。
}

// --- 私有成员函数 ---

// 定义数据库的表结构（Schema）
void DatabaseInserter::_initialize_database() {
    // 创建 'days' 表，用于存储每日的元数据。'date' 是主键。
    execute_sql_Inserter(db, "CREATE TABLE IF NOT EXISTS days (date TEXT PRIMARY KEY, status TEXT, sleep TEXT, remark TEXT, getup_time TEXT);", "Create days table");
    // 创建 'time_records' 表，存储具体的时间段记录。'date' 和 'start' 构成复合主键。
    // 'date' 列是外键，关联到 'days' 表的 'date' 列，确保了数据的引用完整性。
    execute_sql_Inserter(db, "CREATE TABLE IF NOT EXISTS time_records (date TEXT, start TEXT, end TEXT, project_path TEXT, duration INTEGER, PRIMARY KEY (date, start), FOREIGN KEY (date) REFERENCES days(date));", "Create time_records table");
    // 创建 'parent_child' 表，用于存储项目层级关系。
    execute_sql_Inserter(db, "CREATE TABLE IF NOT EXISTS parent_child (child TEXT PRIMARY KEY, parent TEXT);", "Create parent_child table");
}

// 预编译SQL插入语句以提高性能
void DatabaseInserter::_prepare_statements() {
    // 准备 'days' 表的插入语句
    const char* insert_day_sql = "INSERT OR REPLACE INTO days (date, status, sleep, remark, getup_time) VALUES (?, ?, ?, ?, ?);";
    if (sqlite3_prepare_v2(db, insert_day_sql, -1, &stmt_insert_day, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare day insert statement.");
    }

    // 准备 'time_records' 表的插入语句
    const char* insert_record_sql = "INSERT OR REPLACE INTO time_records (date, start, end, project_path, duration) VALUES (?, ?, ?, ?, ?);";
    if (sqlite3_prepare_v2(db, insert_record_sql, -1, &stmt_insert_record, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare time record insert statement.");
    }

    // 准备 'parent_child' 表的插入语句。'OR IGNORE' 表示如果主键已存在，则忽略此次插入。
    const char* insert_parent_child_sql = "INSERT OR IGNORE INTO parent_child (child, parent) VALUES (?, ?);";
    if (sqlite3_prepare_v2(db, insert_parent_child_sql, -1, &stmt_insert_parent_child, nullptr) != SQLITE_OK) {
        throw std::runtime_er