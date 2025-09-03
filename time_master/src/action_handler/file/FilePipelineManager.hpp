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
    explicit FilePipelineManager(const AppConfig& config, const fs::path& output_root);

    // --- 高层接口 ---
    std::optional<fs::path> run(const std::string& input_path);

    // --- [修改] 将流水线的各个阶段设为公有，以支持分步调用 ---
    // [核心修改] 为 collectFiles 方法添加一个扩展名参数，并提供默认值
    bool collectFiles(const std::string& input_path, const std::string& extension = ".txt");
    bool validateSourceFiles();
    bool convertFiles();
    bool validateOutputFiles(bool enable_day_count_check);

private:
    // --- 辅助函数 ---
    void printTimingStatistics(const std::string& operation_name, double total_time_ms) const;

    // --- 状态成员 ---
    const AppConfig& app_config_;
    LogProcessor processor_;
    fs::path output_root_;
    fs::path input_root_;
    std::vector<fs::path> files_to_process_;
    std::map<fs::path, fs::path> source_to_output_map_;
};

#endif // FILE_PIPELINE_MANAGER_HPP