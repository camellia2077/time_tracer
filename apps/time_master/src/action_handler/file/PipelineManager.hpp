// action_handler/file/PipelineManager.hpp
#ifndef PIPELINE_MANAGER_HPP
#define PIPELINE_MANAGER_HPP

#include <string>
#include <vector>
#include <map>
#include <filesystem>
#include <optional>
#include "common/AppConfig.hpp"
#include "reprocessing/LogProcessor.hpp"

namespace fs = std::filesystem;

class PipelineManager {
public:
    explicit PipelineManager(const AppConfig& config, const fs::path& output_root);

    std::optional<fs::path> run(const std::string& input_path);

    bool collectFiles(const std::string& input_path, const std::string& extension = ".txt");
    bool validateSourceFiles();
    
    // --- [核心修改] 返回值变回 bool，表示转换-输出流程是否成功 ---
    bool convertFiles(); 

    bool validateOutputFiles(bool enable_day_count_check);

private:
    void printTimingStatistics(const std::string& operation_name, double total_time_ms) const;

    const AppConfig& app_config_;
    LogProcessor processor_;
    fs::path output_root_;
    fs::path input_root_;
    std::vector<fs::path> files_to_process_;
    
    // --- [核心修改] ---
    // 不再使用 source_to_output_map_，而是存储所有输出文件的路径
    std::vector<fs::path> generated_output_files_; 
};

#endif // PIPELINE_MANAGER_HPP