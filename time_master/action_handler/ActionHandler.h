#ifndef ACTION_HANDLER_H
#define ACTION_HANDLER_H

// action_handler/ActionHandler.h
#include "reprocessing/LogProcessor.h"
#include <string>
#include <vector>
#include <map>
#include <filesystem>
#include "queries/shared/query_data_structs.h"
#include "queries/shared/ReportFormat.h"

// 前向声明
struct sqlite3;
namespace fs = std::filesystem;

class ActionHandler {
public:
    // ... (public members remain the same) ...
    ActionHandler(const std::string& db_name, const AppConfig& config, const std::string& main_config_path);
    ~ActionHandler();

    // --- 文件预处理的原子操作 ---
    bool collectFiles(const std::string& input_path);
    bool validateSourceFiles();
    bool convertFiles();
    bool validateOutputFiles(bool enable_day_count_check);

    // --- 数据库和完整流水线操作 ---
    void run_database_import(const std::string& processed_path);
    void run_full_pipeline_and_import(const std::string& source_path);

    // --- 查询相关 ---
    std::string run_daily_query(const std::string& date, ReportFormat format) const;
    std::string run_period_query(int days, ReportFormat format) const;
    std::string run_monthly_query(const std::string& month, ReportFormat format) const;
    
    // --- 导出功能 ---
    void run_export_all_daily_reports_query(ReportFormat format) const;
    void run_export_all_monthly_reports_query(ReportFormat format) const;
    void run_export_all_period_reports_query(const std::vector<int>& days_list, ReportFormat format) const;


private:
    // 数据库连接管理
    bool open_database_if_needed() const;
    void close_database();

    // 封装了阶段性总结打印格式
    void printTimingStatistics(const std::string& operation_name, double total_time_ms) const;

    mutable sqlite3* db_;
    std::string db_name_;
    AppConfig app_config_;
    std::string main_config_path_;

    

    // 用于保存文件处理状态的成员
    fs::path input_root_;
    std::vector<fs::path> files_to_process_;
    std::map<fs::path, fs::path> source_to_output_map_;
    LogProcessor processor_;
    // 新增：用于存储导出根路径的成员变量
    std::filesystem::path export_root_path_; 
};

#endif // ACTION_HANDLER_H