#ifndef DATABASE_INSERTER_H
#define DATABASE_INSERTER_H

#include <string>
#include <sqlite3.h>
#include "data_parser.h" // 引入data_parser.h，以便使用其中的数据结构（如DataFileParser）

/**
 * @class DatabaseInserter
 * @brief 负责处理所有数据库写入操作的类。
 *
 * 该类封装了打开数据库、创建表、准备SQL语句以及
 * 将从DataFileParser解析出的数据批量导入数据库的全部逻辑。
 */
class DatabaseInserter {
public:
    /**
     * @brief 构造函数，根据给定的路径打开或创建数据库文件。
     * @param db_path 数据库文件的路径。
     */
    DatabaseInserter(const std::string& db_path);

    /**
     * @brief 析构函数，负责释放资源，如关闭数据库连接。
     */
    ~DatabaseInserter();

    /**
     * @brief 检查数据库是否成功打开。
     * @return 如果数据库连接有效，则返回 true。
     */
    bool is_db_open() const;

    /**
     * @brief 将解析器中的所有数据导入到数据库中。
     * @param parser 包含已解析数据的 DataFileParser 对象。
     */
    void import_data(const DataFileParser& parser);

private:
    sqlite3* db; // 指向SQLite数据库连接的指针。
    
    // --- 预编译的SQL语句指针 ---
    // 预编译可以极大地提高批量插入的性能。
    sqlite3_stmt* stmt_insert_day;          // 用于向 'days' 表插入数据。
    sqlite3_stmt* stmt_insert_record;       // 用于向 'time_records' 表插入数据。
    sqlite3_stmt* stmt_insert_parent_child; // 用于向 'parent_child' 表插入数据。

    /**
     * @brief 初始化数据库，如果表不存在，则创建它们。
     */
    void _initialize_database();

    /**
     * @brief 准备（预编译）所有需要用到的INSERT语句。
     */
    void _prepare_statements();
    
    /**
     * @brief 清理和释放所有预编译的语句。
     */
    void _finalize_statements();
};

/**
 * @brief 一个通用的辅助函数，用于执行不返回结果的SQL语句。
 * @param db 数据库连接指针。
 * @param sql 要执行的SQL语句字符串。
 * @param context_msg 用于在发生错误时提供上下文信息的字符串。
 * @return 如果执行成功，返回 true。
 */
bool execute_sql_Inserter(sqlite3* db, const std::string& sql, const std::string& context_msg = "");

#endif // DATABASE_INSERTER_H