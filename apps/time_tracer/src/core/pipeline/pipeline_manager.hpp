// core/pipeline/pipeline_manager.hpp
#ifndef CORE_PIPELINE_PIPELINE_MANAGER_HPP_
#define CORE_PIPELINE_PIPELINE_MANAGER_HPP_

#include <string>
#include <vector>
#include <filesystem>
#include <optional>

#include "common/config/app_config.hpp"
#include "common/app_options.hpp"
#include "core/pipeline/context/pipeline_context.hpp"

namespace fs = std::filesystem;

namespace core::pipeline {

class PipelineManager {
public:
    explicit PipelineManager(const AppConfig& config, const fs::path& output_root);

    std::optional<PipelineContext> run(const std::string& input_path, const AppOptions& options);

private:
    const AppConfig& app_config_;
    fs::path output_root_;
};

} // namespace core::pipeline

#endif // CORE_PIPELINE_PIPELINE_MANAGER_HPP_