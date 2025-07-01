#ifndef MENU_H
#define MENU_H

#include <string>
#include "LogProcessor.h" 

struct sqlite3;

class Menu {
public:
    /**
     * @brief 构造函数
     * @param db_name 数据库文件名。
     * @param config 一个填充好的 AppConfig 结构体。
     * @param main_config_path 主配置文件(config.json)的绝对路径，用于传递给旧模块。
     */
    explicit Menu(const std::string& db_name, const AppConfig& config, const std::string& main_config_path);

    void run();

private:
    sqlite3* db;
    std::string db_name_;
    AppConfig app_config_; 
    std::string main_config_path_; // 存储主配置文件的路径

    // 私有方法
    void print_menu();
    bool handle_user_choice(int choice);
    bool open_database_if_needed();
    void close_database();
    std::string get_valid_date_input();
    std::string get_valid_month_input();
    void run_log_processor_submenu(); 
};

#endif // MENU_H