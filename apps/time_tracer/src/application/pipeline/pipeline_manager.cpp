// application/pipeline/pipeline_manager.cpp
#include "application/pipeline/pipeline_manager.hpp"

#include <stdexcept>
#include <utility>

#include "application/pipeline/steps/pipeline_stages.hpp"
#include "application/ports/i_converter_config_provider.hpp"
#include "application/ports/i_ingest_input_provider.hpp"
#include "application/ports/i_processed_data_storage.hpp"
#include "application/ports/i_validation_issue_reporter.hpp"
#include "application/ports/logger.hpp"
#include "shared/types/ansi_colors.hpp"

namespace core::pipeline {

namespace {

auto Colorize(std::string_view color, std::string_view message) -> std::string {
  return std::string(color) + std::string(message) +
         std::string(time_tracer::common::colors::kReset);
}

}  // namespace

PipelineManager::PipelineManager(
    fs::path output_root,
    std::shared_ptr<time_tracer::application::ports::IConverterConfigProvider>
        converter_config_provider,
    std::shared_ptr<time_tracer::application::ports::IIngestInputProvider>
        ingest_input_provider,
    std::shared_ptr<time_tracer::application::ports::IProcessedDataStorage>
        processed_data_storage,
    std::shared_ptr<time_tracer::application::ports::IValidationIssueReporter>
        validation_issue_reporter)
    : output_root_(std::move(output_root)),
      converter_config_provider_(std::move(converter_config_provider)),
      ingest_input_provider_(std::move(ingest_input_provider)),
      processed_data_storage_(std::move(processed_data_storage)),
      validation_issue_reporter_(std::move(validation_issue_reporter)) {
  if (!converter_config_provider_ || !ingest_input_provider_ ||
      !processed_data_storage_ || !validation_issue_reporter_) {
    throw std::invalid_argument(
        "PipelineManager dependencies must not be null.");
  }
}

auto PipelineManager::Run(const std::string& input_path,
                          const AppOptions& options)
    -> std::optional<PipelineContext> {
  PipelineContext context(output_root_);
  context.config.input_root = input_path;
  context.config.date_check_mode = options.date_check_mode;
  context.config.save_processed_output = options.save_processed_output;
  context.state.validation_issue_reporter = validation_issue_reporter_;

  time_tracer::application::ports::LogInfo(
      std::string("\n") +
      Colorize(time_tracer::common::colors::kBrightCyan,
               std::string(time_tracer::common::colors::kBold) +
                   "--- Pipeline Execution Started ---"));

  if (!core::pipeline::FileCollector::Execute(context, *ingest_input_provider_,
                                              ".txt")) {
    return std::nullopt;
  }

  if (options.validate_structure || options.convert) {
    try {
      time_tracer::application::ports::LogInfo(Colorize(
          time_tracer::common::colors::kGray, "Loading Configuration..."));

      context.state.converter_config =
          converter_config_provider_->LoadConverterConfig();

    } catch (const std::exception& e) {
      time_tracer::application::ports::LogError(
          Colorize(time_tracer::common::colors::kRed,
                   std::string("[Pipeline] 配置加载失败: ") + e.what()));
      return std::nullopt;
    }
  }

  if (options.validate_structure) {
    time_tracer::application::ports::LogInfo(
        Colorize(time_tracer::common::colors::kCyan,
                 "[STEP] Step: Validating File Structure..."));
    if (!core::pipeline::StructureValidatorStep::Execute(context)) {
      return std::nullopt;
    }
  }

  if (options.convert) {
    if (!ConverterStep::Execute(context)) {
      time_tracer::application::ports::LogError(
          Colorize(time_tracer::common::colors::kRed,
                   "[Pipeline] 转换步骤存在错误，终止流程。"));
      return std::nullopt;
    }

    if (!core::pipeline::LogicLinkerStep::Execute(context)) {
      time_tracer::application::ports::LogError(
          Colorize(time_tracer::common::colors::kRed,
                   "[Pipeline] 跨月数据连接失败，流程终止。"));
      return std::nullopt;
    }
  }

  if (options.validate_logic) {
    time_tracer::application::ports::LogInfo(
        Colorize(time_tracer::common::colors::kCyan,
                 "[STEP] Step: Validating Business Logic..."));
    if (!core::pipeline::LogicValidatorStep::Execute(context)) {
      return std::nullopt;
    }
  }

  if (options.convert && options.save_processed_output) {
    time_tracer::application::ports::LogInfo(
        Colorize(time_tracer::common::colors::kCyan,
                 "[STEP] Step: Saving Validated JSON..."));

    auto new_files = processed_data_storage_->WriteProcessedData(
        context.result.processed_data, context.config.output_root);

    context.state.generated_files.insert(context.state.generated_files.end(),
                                         new_files.begin(), new_files.end());

    if (new_files.empty() && !context.result.processed_data.empty()) {
      time_tracer::application::ports::LogWarn(
          Colorize(time_tracer::common::colors::kYellow,
                   std::string(time_tracer::common::colors::kBold) +
                       "[WARN] Data exists but no files were generated (flush "
                       "failed)."));
    } else {
      time_tracer::application::ports::LogInfo(
          Colorize(time_tracer::common::colors::kBrightGreen,
                   "[OK] JSON data safely persisted to disk."));
    }
  }

  time_tracer::application::ports::LogInfo(
      std::string("\n") +
      Colorize(time_tracer::common::colors::kBrightGreen,
               std::string(time_tracer::common::colors::kBold) +
                   "--- Pipeline Finished Successfully ---"));
  return context;
}

}  // namespace core::pipeline
