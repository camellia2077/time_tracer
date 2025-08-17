// time_master_app/menu.h
#ifndef MENU_H
#define MENU_H

#include <string>
#include <memory> // [新增] 包含 <memory> 以使用 std::unique_ptr
#include "common/AppConfig.h"

// 前向声明处理器类
class FileProcessingHandler;
class ReportGenerationHandler;

class Menu {
public:
    explicit Menu(const std::string& db_name, const AppConfig& config, const std::string& main_config_path);
    ~Menu(); // 析构函数仍然需要声明
    void run();

private:
    // [修改] 使用 std::unique_ptr 管理对象的生命周期
    std::unique_ptr<FileProcessingHandler> file_processing_handler_;
    std::unique_ptr<ReportGenerationHandler> report_generation_handler_;

    // --- Private helper functions ---
    void print_menu();
    bool handle_user_choice(int choice);
    void run_log_processor_submenu();
    void run_full_pipeline_and_import_prompt();
    void run_period_query_prompt();

    // --- Single export functions ---
    void run_export_single_day_report_prompt();
    void run_export_single_month_report_prompt();
    void run_export_single_period_report_prompt();

    // --- Bulk export functions ---
    void run_export_all_period_reports_prompt();
};

#endif // MENU_H
