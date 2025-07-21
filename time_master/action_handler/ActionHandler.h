#ifndef ACTION_HANDLER_H
#define ACTION_HANDLER_H

#include "LogProcessor.h"
#include <string>
#include <vector>
#include <map>
#include <filesystem>
#include <functional> // [新增] For std::function
#include <optional>   // [新增] For std::optional
#include "report_generators/_shared/query_data_structs.h"
#include "report_generators/_shared/ReportFormat.h"

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
    // [新增] Helper struct for report format details
    struct ReportFormatDetails {
        std::string dir_name;
        std::string extension;
    };

    // [新增] Extracts report format details, returns nullopt for unsupported formats.
    std::optional<ReportFormatDetails> get_report_format_details(ReportFormat format) const;

    // [新增] Executes the file export task, handling exceptions and console output.
    void execute_export_task(const std::string& report_type_name_singular,
                             const fs::path& export_root_path,
                             const std::function<int()>& file_writing_lambda) const;

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
};

#endif // ACTION_HANDLER_H