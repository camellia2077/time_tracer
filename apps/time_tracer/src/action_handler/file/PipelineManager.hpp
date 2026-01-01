// action_handler/file/PipelineManager.hpp
#ifndef PIPELINE_MANAGER_HPP
#define PIPELINE_MANAGER_HPP

#include <string>
#include <vector>
#include <filesystem>
#include <optional>
#include "common/AppConfig.hpp" // 确保这里包含了 AppOptions 的定义
#include "reprocessing/validator/common/ValidatorUtils.hpp"

namespace fs = std::filesystem;

class PipelineManager {
public:
    explicit PipelineManager(const AppConfig& config, const fs::path& output_root);

    // [修改] run 方法现在接收 AppOptions，以便灵活控制流程
    std::optional<fs::path> run(const std::string& input_path, const AppOptions& options);

    // 旧接口可以删除或保留为私有辅助函数，但为了避免误用，建议直接删除
    // bool collectFiles(...) 
    // bool validateSourceFiles() ...
private:
    const AppConfig& app_config_;
    fs::path output_root_;
};

#endif // PIPELINE_MANAGER_HPP