#ifndef ACTION_HANDLER_H
#define ACTION_HANDLER_H

#include "LogProcessor.h"
#include <string>
#include <vector>
#include <map>
#include <filesystem>

// 前向声明
struct sqlite3;
namespace fs = std::filesystem;

class ActionHandler {
public:
    ActionHandler(const std::string& db_name, const AppConfig& config, const std::string& main_config_path);
    ~ActionHandler();

    // --- [REFACTORED] 文件预处理的原子操作 ---
    /**
     * @brief 收集输入路径下的所有待处理文件，并存入内部状态。
     * @param input_path 用户提供的文件或目录路径。
     * @return 如果成功收集到文件则返回 true。
     */
    bool collectFiles(const std::string& input_path);

    /**
     * @brief 验证所有已收集源文件的格式。
     * @return 如果所有文件都验证成功则返回 true。
     */
    bool validateSourceFiles();

    /**
     * @brief 转换所有已收集的源文件，并将结果保存在 "Processed_" 目录下。
     * 此方法会填充内部的源文件到输出文件的路径映射。
     * @return 如果所有文件都转换成功则返回 true。
     */
    bool convertFiles();

    /**
     * @brief 验证所有已转换输出文件的格式。
     * @param enable_day_count_check 是否启用月份天数完整性检查。
     * @return 如果所有文件都验证成功则返回 true。
     */
    bool validateOutputFiles(bool enable_day_count_check);


    // --- 数据库和完整流水线操作 ---
    void run_database_import(const std::string& processed_path);
    void run_full_pipeline_and_import(const std::string& source_path);

    // --- 查询相关 ---
    std::string run_daily_query(const std::string& date);
    std::string run_period_query(int days);
    std::string run_monthly_query(const std::string& month);
    

private:
    // 数据库连接管理
    bool open_database_if_needed();
    void close_database();

    // --- 封装了阶段性总结打印格式
    void printTimingStatistics(const std::string& operation_name, double total_time_ms) const;

    sqlite3* db_;
    std::string db_name_;
    AppConfig app_config_;
    std::string main_config_path_;

    // --- [NEW] 用于保存文件处理状态的成员 ---
    fs::path input_root_;
    std::vector<fs::path> files_to_process_;
    std::map<fs::path, fs::path> source_to_output_map_;
    LogProcessor processor_; // 将 LogProcessor 作为成员，以便在多个方法中重用
};

#endif // ACTION_HANDLER_H