// application/pipeline/context/pipeline_context.hpp
#ifndef APPLICATION_PIPELINE_CONTEXT_PIPELINE_CONTEXT_H_
#define APPLICATION_PIPELINE_CONTEXT_PIPELINE_CONTEXT_H_

#include <filesystem>
#include <map>
#include <memory>
#include <vector>

#include "application/dto/ingest_input_model.hpp"
#include "domain/model/daily_log.hpp"
#include "domain/types/converter_config.hpp"
#include "domain/types/date_check_mode.hpp"

namespace fs = std::filesystem;

namespace tracer_core::application::ports {
class IValidationIssueReporter;
}  // namespace tracer_core::application::ports

namespace core::pipeline {

/**
 * @brief Pipeline 任务的运行时配置 (Runtime Config)
 */
struct PipelineRunConfig {
  fs::path input_root;
  fs::path output_root;

  DateCheckMode date_check_mode = DateCheckMode::kNone;
  bool save_processed_output = false;

  explicit PipelineRunConfig(fs::path out) : output_root(std::move(out)) {}
};

/**
 * @brief Pipeline 运行时的可变状态 (Mutable State)
 */
struct PipelineState {
  std::vector<tracer_core::application::dto::IngestInputModel> ingest_inputs;
  std::vector<fs::path> generated_files;
  std::shared_ptr<tracer_core::application::ports::IValidationIssueReporter>
      validation_issue_reporter;

  // 运行时加载的 ConverterConfig (Struct)
  ConverterConfig converter_config;
};

/**
 * @brief Pipeline 的业务数据产出 (Result)
 */
struct PipelineResult {
  // 转换后的核心业务数据
  std::map<std::string, std::vector<DailyLog>> processed_data;
};

/**
 * @brief Pipeline 上下文
 */
class PipelineContext {
 public:
  PipelineRunConfig config;
  PipelineState state;
  PipelineResult result;

  explicit PipelineContext(const fs::path& out_root) : config(out_root) {}
};

}  // namespace core::pipeline

#endif  // APPLICATION_PIPELINE_CONTEXT_PIPELINE_CONTEXT_H_
