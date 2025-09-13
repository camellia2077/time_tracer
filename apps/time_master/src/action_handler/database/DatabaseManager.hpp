// action_handler/database/DatabaseManager.hpp
#ifndef DATABASE_MANAGER_HPP
#define DATABASE_MANAGER_HPP

#include <string>
#include <filesystem>
#include <iostream>
#include "common/AnsiColors.hpp" // For colored console output

// 前向声明
struct sqlite3;

/**
 * @brief 数据库管理器类
 * * 负责数据库的连接和关闭。
 * 封装了底层的 sqlite3 连接句柄。
 */
class DatabaseManager {
public:
    /**
     * @brief 构造函数。
     * @param db_name 数据库文件路径。
     */
    DatabaseManager(const std::string& db_name);

    /**
     * @brief 析构函数。
     * * 确保在对象销毁时关闭数据库连接。
     */
    ~DatabaseManager();

    // 禁止拷贝和赋值，因为该类管理着一个唯一的资源（数据库连接）。
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    /**
     * @brief 如果需要，则打开数据库连接。
     * * 如果连接已打开，则此方法不执行任何操作。
     * @return 如果连接成功打开或已打开，则返回 true；否则返回 false。
     */
    bool open_database_if_needed();

    /**
     * @brief 关闭数据库连接。
     * * 如果连接已打开，则关闭它并释放资源。
     */
    void close_database();

    /**
     * @brief 获取底层的 sqlite3 连接句柄。
     * @return sqlite3 连接指针。
     */
    sqlite3* get_db_connection() const;

private:
    std::string db_name_;
    sqlite3* db_;
};

#endif // DATABASE_MANAGER_HPP