#ifndef ACTION_HANDLER_H
#define ACTION_HANDLER_H

#include "LogProcessor.h"
#include <string>
#include <vector>
#include <map>
#include <filesystem>
#include "report_generators/_shared/query_data_structs.h" // [新增]

// 前向声明
struct sqlite3;
namespace fs = std::filesystem;

class ActionHandler {
public:
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
    std::string run_daily_query(const std::string& date) const;
    std::string run_period_query(int days) const;
    std::string run_monthly_query(const std::string& month) const;
    
    // --- 导出功能 ---
    /**
     * @brief 导出所有日报，每个日报存为一个独立的 .md 文件。
     */
    void run_export_all_daily_reports_query() const;

    /**
     * @brief 导出所有月报，每个月合并成一个 .md 文件。
     */
    void run_export_all_monthly_reports_query() const;

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
};

#endif // ACTION_HANDLER_H
