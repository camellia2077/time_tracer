// time_master_app/menu.hpp
#ifndef MENU_HPP
#define MENU_HPP

#include <string>
#include <memory>
#include "common/AppConfig.hpp"

// 前向声明处理器类
class FileProcessingHandler;
class ReportGenerationHandler;

class Menu {
public:
    explicit Menu(const std::string& db_name, const AppConfig& config, const std::string& main_config_path);
    ~Menu();
    void run();

private:
    std::unique_ptr<FileProcessingHandler> file_processing_handler_;
    std::unique_ptr<ReportGenerationHandler> report_generation_handler_;

    // --- Private helper functions ---
    void print_menu();
    bool handle_user_choice(int choice);
    void run_log_processor_submenu();
    void run_full_pipeline_and_import_prompt();
    void run_query_submenu();
    void run_export_submenu();
};

#endif // MENU_HPP