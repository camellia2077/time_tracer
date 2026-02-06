// application/pipeline/pipeline_manager.hpp
#ifndef APPLICATION_PIPELINE_PIPELINE_MANAGER_H_
#define APPLICATION_PIPELINE_PIPELINE_MANAGER_H_

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "application/pipeline/context/pipeline_context.hpp"
#include "common/app_options.hpp"
#include "common/config/app_config.hpp"

namespace fs = std::filesystem;

namespace core::pipeline {

class PipelineManager {
 public:
  explicit PipelineManager(const AppConfig& config, fs::path output_root);

  [[nodiscard]] auto Run(const std::string& input_path,
                         const AppOptions& options)
      -> std::optional<PipelineContext>;

 private:
  const AppConfig& app_config_;
  fs::path output_root_;
};

}  // namespace core::pipeline

#endif  // APPLICATION_PIPELINE_PIPELINE_MANAGER_H_
