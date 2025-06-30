#ifndef MENU_H
#define MENU_H

#include <string>


struct sqlite3;

class Menu {
public:
    /**
     * @brief 构造函数，接收数据库名和主配置文件路径。
     * @param db_name 数据库文件名。
     * @param config_path 主配置文件 (config.json) 的路径。
     */
    // 【修改】将构造函数改回双参数版本，以匹配 menu.cpp 的定义和 main.cpp 的调用
    explicit Menu(const std::string& db_name, const std::string& config_path);

    void run();

private:
    sqlite3* db;
    std::string db_name_;
    std::string config_path_; 

    void print_menu();
    bool handle_user_choice(int choice);
    bool open_database_if_needed();
    void close_database();
    std::string get_valid_date_input();
    std::string get_valid_month_input();
    void process_files_option();

};

#endif // MENU_H