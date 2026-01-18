// core/file/pipeline_manager.hpp

// core/file/PipelineManager.hpp
#ifndef CORE_FILE_PIPELINE_MANAGER_HPP_
#define CORE_FILE_PIPELINE_MANAGER_HPP_

#include <string>
#include <vector>
#include <filesystem>
#include <optional>

#include "common/config/app_config.hpp"
#include "common/app_options.hpp"

#include "validator/common/ValidatorUtils.hpp"
#include "core/file/pipeline_context.hpp"

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

#endif // CORE_FILE_PIPELINE_MANAGER_HPP_
