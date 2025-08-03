// time_master_app/menu.h
#ifndef MENU_H
#define MENU_H

#include <string>
#include "common/AppConfig.h"

// [修改] 前向声明新的处理器类
class FileProcessingHandler;
class ReportGenerationHandler;
class LogProcessorMenu; // LogProcessorMenu 也需要前向声明

class Menu {
public:
    explicit Menu(const std::string& db_name, const AppConfig& config, const std::string& main_config_path);
    ~Menu();
    void run();

private:
    // [修改] 使用新的、职责更明确的处理器
    FileProcessingHandler* file_processing_handler_;
    ReportGenerationHandler* report_generation_handler_;

    // --- 私有辅助函数 ---
    void print_menu();
    bool handle_user_choice(int choice);
    void run_log_processor_submenu();
    void run_full_pipeline_and_import_prompt();
    void run_period_query_prompt();

    // --- 单独导出功能的函数声明 ---
    void run_export_single_day_report_prompt();
    void run_export_single_month_report_prompt();
    void run_export_single_period_report_prompt();

    // --- 批量导出方法 ---
    void run_export_all_period_reports_prompt();
};

#endif // MENU_H