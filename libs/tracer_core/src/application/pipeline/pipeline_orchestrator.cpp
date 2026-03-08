#if TT_ENABLE_CPP20_MODULES
module;

#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "application/ports/i_converter_config_provider.hpp"
#include "application/ports/i_ingest_input_provider.hpp"
#include "application/ports/i_processed_data_storage.hpp"
#include "application/ports/i_validation_issue_reporter.hpp"
#include "application/ports/logger.hpp"

module tracer.core.application.pipeline.orchestrator;

import tracer.core.application.pipeline.stages;
import tracer.core.domain.types.app_options;
import tracer.core.shared.ansi_colors;

using tracer::core::domain::modtypes::AppOptions;
namespace modcolors = tracer::core::shared::modcolors;

#else

#include "application/pipeline/pipeline_orchestrator.hpp"
#include "application/pipeline/pipeline_stages.hpp"

#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "application/ports/i_converter_config_provider.hpp"
#include "application/ports/i_ingest_input_provider.hpp"
#include "application/ports/i_processed_data_storage.hpp"
#include "application/ports/i_validation_issue_reporter.hpp"
#include "application/ports/logger.hpp"
#include "domain/types/app_options.hpp"
#include "shared/types/ansi_colors.hpp"

using ::AppOptions;
namespace modcolors = tracer_core::common::colors;

#endif

namespace tracer::core::application::pipeline {

namespace {

auto Colorize(std::string_view color, std::string_view message) -> std::string {
  return std::string(color) + std::string(message) +
         std::string(modcolors::kReset);
}

}  // namespace

PipelineOrchestrator::PipelineOrchestrator(
    fs::path output_root,
    std::shared_ptr<tracer_core::application::ports::IConverterConfigProvider>
        converter_config_provider,
    std::shared_ptr<tracer_core::application::ports::IIngestInputProvider>
        ingest_input_provider,
    std::shared_ptr<tracer_core::application::ports::IProcessedDataStorage>
        processed_data_storage,
    std::shared_ptr<tracer_core::application::ports::IValidationIssueReporter>
        validation_issue_reporter)
    : output_root_(std::move(output_root)),
      converter_config_provider_(std::move(converter_config_provider)),
      ingest_input_provider_(std::move(ingest_input_provider)),
      processed_data_storage_(std::move(processed_data_storage)),
      validation_issue_reporter_(std::move(validation_issue_reporter)) {
  if (!converter_config_provider_ || !ingest_input_provider_ ||
      !processed_data_storage_ || !validation_issue_reporter_) {
    throw std::invalid_argument(
        "PipelineOrchestrator dependencies must not be null.");
  }
}

auto PipelineOrchestrator::Run(const AppOptions& options)
    -> std::optional<PipelineSession> {
  PipelineSession session(output_root_);
  session.config.input_root = options.input_path;
  session.config.date_check_mode = options.date_check_mode;
  session.config.save_processed_output = options.save_processed_output;
  session.state.validation_issue_reporter = validation_issue_reporter_;

  tracer_core::application::ports::LogInfo(
      std::string("\n") +
      Colorize(modcolors::kBrightCyan,
               std::string(modcolors::kBold) +
                   "--- Pipeline Execution Started ---"));

  if (!InputCollectionStage::Execute(session, *ingest_input_provider_, ".txt")) {
    return std::nullopt;
  }

  if (options.validate_structure || options.convert) {
    try {
      tracer_core::application::ports::LogInfo(
          Colorize(modcolors::kGray, "Loading Configuration..."));

      session.state.converter_config =
          converter_config_provider_->LoadConverterConfig();

    } catch (const std::exception& e) {
      tracer_core::application::ports::LogError(
          Colorize(modcolors::kRed,
                   std::string("[Pipeline] 配置加载失败: ") + e.what()));
      return std::nullopt;
    }
  }

  if (options.validate_structure) {
    tracer_core::application::ports::LogInfo(
        Colorize(modcolors::kCyan, "[STEP] Step: Validating File Structure..."));
    if (!StructureValidationStage::Execute(session)) {
      return std::nullopt;
    }
  }

  if (options.convert) {
    if (!ConversionStage::Execute(session)) {
      tracer_core::application::ports::LogError(
          Colorize(modcolors::kRed,
                   "[Pipeline] 转换步骤存在错误，终止流程。"));
      return std::nullopt;
    }

    if (!CrossMonthLinkStage::Execute(session)) {
      tracer_core::application::ports::LogError(
          Colorize(modcolors::kRed,
                   "[Pipeline] 跨月数据连接失败，流程终止。"));
      return std::nullopt;
    }
  }

  if (options.validate_logic) {
    tracer_core::application::ports::LogInfo(
        Colorize(modcolors::kCyan, "[STEP] Step: Validating Business Logic..."));
    if (!LogicValidationStage::Execute(session)) {
      return std::nullopt;
    }
  }

  if (options.convert && options.save_processed_output) {
    tracer_core::application::ports::LogInfo(
        Colorize(modcolors::kCyan, "[STEP] Step: Saving Validated JSON..."));

    auto new_files = processed_data_storage_->WriteProcessedData(
        session.result.processed_data, session.config.output_root);

    session.state.generated_files.insert(session.state.generated_files.end(),
                                         new_files.begin(), new_files.end());

    if (new_files.empty() && !session.result.processed_data.empty()) {
      tracer_core::application::ports::LogWarn(
          Colorize(modcolors::kYellow,
                   std::string(modcolors::kBold) +
                       "[WARN] Data exists but no files were generated (flush "
                       "failed)."));
    } else {
      tracer_core::application::ports::LogInfo(
          Colorize(modcolors::kBrightGreen,
                   "[OK] JSON data safely persisted to disk."));
    }
  }

  tracer_core::application::ports::LogInfo(
      std::string("\n") +
      Colorize(modcolors::kBrightGreen,
               std::string(modcolors::kBold) +
                   "--- Pipeline Finished Successfully ---"));
  return session;
}

}  // namespace tracer::core::application::pipeline
