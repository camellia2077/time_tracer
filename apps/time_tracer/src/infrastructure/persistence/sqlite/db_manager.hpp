// infrastructure/persistence/sqlite/db_manager.hpp
#ifndef INFRASTRUCTURE_PERSISTENCE_SQLITE_DB_MANAGER_H_
#define INFRASTRUCTURE_PERSISTENCE_SQLITE_DB_MANAGER_H_

#include <filesystem>
#include <iostream>
#include <string>

#include "shared/types/ansi_colors.hpp"  // For colored console output

// 前向声明
struct sqlite3;

/**
 * @brief 数据库管理器类
 * * 负责数据库的连接和关闭。
 * 封装了底层的 sqlite3 连接句柄。
 */
class DBManager {
 public:
  /**
   * @brief 构造函数。
   * @param db_name 数据库文件路径。
   */
  DBManager(std::string db_name);

  /**
   * @brief 析构函数。
   * * 确保在对象销毁时关闭数据库连接。
   */
  ~DBManager();

  // 禁止拷贝和赋值，因为该类管理着一个唯一的资源（数据库连接）。
  DBManager(const DBManager&) = delete;
  auto operator=(const DBManager&) -> DBManager& = delete;

  /**
   * @brief 如果需要，则打开数据库连接。
   * * 如果连接已打开，则此方法不执行任何操作。
   * @return 如果连接成功打开或已打开，则返回 true；否则返回 false。
   */
  [[nodiscard]] auto OpenDatabaseIfNeeded() -> bool;

  /**
   * @brief 关闭数据库连接。
   * * 如果连接已打开，则关闭它并释放资源。
   */
  auto CloseDatabase() -> void;

  /**
   * @brief 获取底层的 sqlite3 连接句柄。
   * @return sqlite3 连接指针。
   */
  [[nodiscard]] auto GetDbConnection() const -> sqlite3*;

 private:
  std::string db_name_;
  sqlite3* db_ = nullptr;
};

#endif  // INFRASTRUCTURE_PERSISTENCE_SQLITE_DB_MANAGER_H_
