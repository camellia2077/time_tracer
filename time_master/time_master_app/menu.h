#ifndef MENU_H
#define MENU_H

#include <string>
#include "LogProcessor.h" // For AppConfig

// 前向声明，以避免在头文件中包含 ActionHandler.h
class ActionHandler;

class Menu {
public:
    explicit Menu(const std::string& db_name, const AppConfig& config, const std::string& main_config_path);
    ~Menu(); // 添加析构函数来管理 ActionHandler
    void run();

private:
    // Menu 不再直接管理数据库或配置，所有操作通过 ActionHandler
    ActionHandler* action_handler_; // 使用指针，因为 ActionHandler 管理资源

    // --- 私有辅助函数 ---
    void print_menu();
    bool handle_user_choice(int choice);
    void run_log_processor_submenu();
    void run_full_pipeline_and_import_prompt();
    void run_period_query_prompt(); // [新增]
    void run_export_period_reports_prompt(); // [新增]

    // --- 用户输入处理 ---
    std::string get_valid_path_input(const std::string& prompt_message);
    std::string get_valid_date_input();
    std::string get_valid_month_input();
};

#endif // MENU_H