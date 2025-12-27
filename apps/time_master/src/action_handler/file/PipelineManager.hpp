
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
// [新增] 引入 ValidatorUtils
#include "reprocessing/validator/common/ValidatorUtils.hpp"

namespace fs = std::filesystem;

class PipelineManager {
public:
    explicit PipelineManager(const AppConfig& config, const fs::path& output_root);

    // [修改] run 方法接收 date_check_mode 参数
    std::optional<fs::path> run(const std::string& input_path, DateCheckMode date_check_mode);

    bool collectFiles(const std::string& input_path, const std::string& extension = ".txt");
    bool validateSourceFiles();
    bool convertFiles(); 

    // [修改] validateOutputFiles 接收 date_check_mode 参数
    bool validateOutputFiles(DateCheckMode date_check_mode);

private:
    void printTimingStatistics(const std::string& operation_name, double total_time_ms) const;

    const AppConfig& app_config_;
    LogProcessor processor_;
    fs::path output_root_;
    fs::path input_root_;
    std::vector<fs::path> files_to_process_;
    std::vector<fs::path> generated_output_files_; 
};

#endif // PIPELINE_MANAGER_HPP
