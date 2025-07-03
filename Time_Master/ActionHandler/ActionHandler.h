#ifndef ACTION_HANDLER_H
#define ACTION_HANDLER_H

#include "LogProcessor.h"
#include <string>

// 前向声明，避免在头文件中包含 sqlite3.h
struct sqlite3;

class ActionHandler {
public:
    ActionHandler(const std::string& db_name, const AppConfig& config, const std::string& main_config_path);
    ~ActionHandler();

    // --- 所有核心业务逻辑的方法 ---

    // 查询相关
    void run_daily_query(const std::string& date);
    void run_period_query(int days);
    void run_monthly_query(const std::string& month);
    
    // 文件预处理（对应旧的 LogProcessor 子菜单）
    void run_log_processing(const AppOptions& options);
    
    // 数据库导入（对应旧的子菜单选项 7）
    void run_database_import(const std::string& processed_path);
    
    // 完整流水线处理
    void run_full_pipeline_and_import(const std::string& source_path);

private:
    // 数据库连接管理
    bool open_database_if_needed();
    void close_database();

    sqlite3* db_;
    std::string db_name_;
    AppConfig app_config_;
    std::string main_config_path_;
};

#endif // ACTION_HANDLER_H