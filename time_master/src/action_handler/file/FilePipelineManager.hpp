// time_master/action_handler/file/FilePipelineManager.hpp

#ifndef FILE_PIPELINE_MANAGER_HPP
#define FILE_PIPELINE_MANAGER_HPP

#include <string>
#include <vector>
#include <map>
#include <filesystem>
#include <optional>
#include "common/AppConfig.hpp" // 通用配置头文件
#include "reprocessing/LogProcessor.hpp" 


namespace fs = std::filesystem;

class FilePipelineManager {
public:
    explicit FilePipelineManager(const AppConfig& config);

    // --- 高层接口 ---
    std::optional<fs::path> run(const std::string& input_path);

    // --- [修改] 将流水线的各个阶段设为公有，以支持分步调用 ---
    bool collectFiles(const std::string& input_path);
    bool validateSourceFiles();
    bool convertFiles();
    bool validateOutputFiles(bool enable_day_count_check);

private:
    // --- 辅助函数 ---
    void printTimingStatistics(const std::string& operation_name, double total_time_ms) const;

    // --- 状态成员 ---
    fs::path input_root_;
    std::vector<fs::path> files_to_process_;
    std::map<fs::path, fs::path> source_to_output_map_;
    LogProcessor processor_;
    const AppConfig& app_config_;
};

#endif // FILE_PIPELINE_MANAGER_HPP