#ifndef ACTION_HANDLER_H
#define ACTION_HANDLER_H

#include "reprocessing/LogProcessor.h"
#include <string>
#include <vector>
#include <map>
#include <filesystem>
#include <memory> // 用于智能指针
#include "queries/shared/query_data_structs.h"
#include "queries/shared/ReportFormat.h"

// 前向声明
namespace fs = std::filesystem;

class DatabaseManager;      // 管理数据库连接
class ReportExporter;       // 管理报告导出
class DirectQueryManager;   // 新增: 管理直接查询

/**
 * @brief 行为协调器 (重构后)
 * @details 作为应用功能的高层门面 (Facade)，负责协调各个子系统
 * (如数据库、报告导出、文件处理流水线等) 来完成任务。
 */
class ActionHandler {
public:
    ActionHandler(const std::string& db_name, const AppConfig& config, const std::string& main_config_path);
    ~ActionHandler();

    // --- 直接查询功能 (用于在控制台显示) ---
    std::string run_daily_query(const std::string& date, ReportFormat format);
    std::string run_monthly_query(const std::string& month, ReportFormat format);
    std::string run_period_query(int days, ReportFormat format);

    // --- 文件预处理流水线操作 ---
    bool collectFiles(const std::string& input_path);
    bool validateSourceFiles();
    bool convertFiles();
    bool validateOutputFiles(bool enable_day_count_check);

    // --- 数据库导入和完整流水线操作 ---
    void run_database_import(const std::string& processed_path);
    void run_full_pipeline_and_import(const std::string& source_path);

    // --- 报告导出功能 (委托给 ReportExporter) ---
    void run_export_single_day_report(const std::string& date, ReportFormat format);
    void run_export_single_month_report(const std::string& month, ReportFormat format);
    void run_export_single_period_report(int days, ReportFormat format);
    void run_export_all_daily_reports_query(ReportFormat format);
    void run_export_all_monthly_reports_query(ReportFormat format);
    void run_export_all_period_reports_query(const std::vector<int>& days_list, ReportFormat format);

private:
    void printTimingStatistics(const std::string& operation_name, double total_time_ms) const;
    // --- 新增: 私有辅助函数，用于获取子管理器 ---
    DirectQueryManager* get_direct_query_manager();
    ReportExporter* get_report_exporter();

    // 子系统管理器
    std::unique_ptr<DatabaseManager> db_manager_;
    std::unique_ptr<ReportExporter> report_exporter_; // 将被懒加载（需要时再创建）
    std::unique_ptr<DirectQueryManager> direct_query_manager_; // 新增: 直接查询管理器，同样懒加载

    // 配置信息
    AppConfig app_config_;
    std::string main_config_path_;
    std::string db_name_; // 新增: 存储数据库名称以供使用
    fs::path export_root_path_;

    // 用于文件处理流水线的状态成员
    fs::path input_root_;
    std::vector<fs::path> files_to_process_;
    std::map<fs::path, fs::path> source_to_output_map_;
    LogProcessor processor_;
};

#endif // ACTION_HANDLER_H