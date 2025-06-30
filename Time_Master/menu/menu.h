// --- START OF FILE Menu/menu.h ---

#ifndef MENU_H
#define MENU_H

#include <string>

struct sqlite3;

class Menu {
public:
    /**
     * @brief 构造函数，接收数据库名和配置文件路径。
     */
    explicit Menu(const std::string& db_name, const std::string& config_path);

    void run();

private:
    sqlite3* db;
    std::string db_name_;
    std::string config_path_; // 【新增】存储 config.json 的路径

    void print_menu();
    bool handle_user_choice(int choice);
    bool open_database_if_needed();
    void close_database();
    std::string get_valid_date_input();
    std::string get_valid_month_input();
    void process_files_option(); // 将处理逻辑移到私有方法中
};

#endif // MENU_H