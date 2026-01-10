// action_handler/file/PipelineManager.hpp
#ifndef PIPELINE_MANAGER_HPP
#define PIPELINE_MANAGER_HPP

#include <string>
#include <vector>
#include <filesystem>
#include <optional>
#include "common/AppConfig.hpp"
#include "reprocessing/validator/common/ValidatorUtils.hpp"
// [新增] 必须包含 PipelineContext 定义（或前向声明），因为返回值变了
#include "action_handler/file/PipelineContext.hpp"

namespace fs = std::filesystem;

class PipelineManager {
public:
    explicit PipelineManager(const AppConfig& config, const fs::path& output_root);

    // [修改] run 方法现在返回 PipelineContext，以便携带内存数据
    std::optional<PipelineContext> run(const std::string& input_path, const AppOptions& options);

private:
    const AppConfig& app_config_;
    fs::path output_root_;
};

#endif // PIPELINE_MANAGER_HPP