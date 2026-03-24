module;

#include <stdexcept>
#include <string>
#include <utility>

#include "application/ports/pipeline/i_converter_config_provider.hpp"
#include "application/ports/pipeline/i_ingest_input_provider.hpp"
#include "application/ports/pipeline/i_processed_data_storage.hpp"
#include "application/ports/pipeline/i_validation_issue_reporter.hpp"
#include "application/runtime_bridge/logger.hpp"

module tracer.core.application.pipeline.orchestrator;

import tracer.core.application.pipeline.stages;
import tracer.core.domain.types.app_options;
using tracer::core::domain::types::AppOptions;

namespace tracer::core::application::pipeline {
namespace {

[[nodiscard]] auto BuildStructureValidationStepLabel(
    bool structure_validation_blocks_conversion) -> std::string_view {
  if (structure_validation_blocks_conversion) {
    return "[STEP] Step: Pre-validating File Structure before Conversion...";
  }
  return "[STEP] Step: Validating File Structure...";
}

[[nodiscard]] auto BuildStructureValidationFailureMessage(
    bool structure_validation_blocks_conversion) -> std::string_view {
  if (structure_validation_blocks_conversion) {
    return "[Pipeline] 转换前结构预检失败，已跳过转换阶段。";
  }
  return "[Pipeline] 文件结构校验失败，流程终止。";
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
  const bool kRunStructureValidationBeforeConversion =
      options.convert && options.run_structure_validation_before_conversion;
  const bool kRunStructureValidation =
      options.validate_structure || kRunStructureValidationBeforeConversion;
  PipelineSession session(output_root_);
  session.config.input_root = options.input_path;
  session.config.date_check_mode = options.date_check_mode;
  session.config.structure_validation_blocks_conversion =
      options.convert && kRunStructureValidation;
  session.config.save_processed_output = options.save_processed_output;
  session.state.validation_issue_reporter = validation_issue_reporter_;

  tracer_core::application::runtime_bridge::LogInfo(
      "\n--- Pipeline Execution Started ---");

  if (!InputCollectionStage::Execute(session, *ingest_input_provider_, ".txt")) {
    return std::nullopt;
  }

  if (options.validate_structure || options.convert) {
    try {
      tracer_core::application::runtime_bridge::LogInfo("Loading Configuration...");

      session.state.converter_config =
          converter_config_provider_->LoadConverterConfig();

    } catch (const std::exception& e) {
      tracer_core::application::runtime_bridge::LogError(
          std::string("[Pipeline] 配置加载失败: ") + e.what());
      return std::nullopt;
    }
  }

  if (kRunStructureValidation) {
    tracer_core::application::runtime_bridge::LogInfo(
        BuildStructureValidationStepLabel(
            session.config.structure_validation_blocks_conversion));
    if (!StructureValidationStage::Execute(session)) {
      tracer_core::application::runtime_bridge::LogError(
          BuildStructureValidationFailureMessage(
              session.config.structure_validation_blocks_conversion));
      return std::nullopt;
    }
  }

  if (options.convert) {
    if (!ConversionStage::Execute(session)) {
      tracer_core::application::runtime_bridge::LogError(
          "[Pipeline] 转换阶段失败，流程终止。");
      return std::nullopt;
    }

    if (!CrossMonthLinkStage::Execute(session)) {
      tracer_core::application::runtime_bridge::LogError(
          "[Pipeline] 跨月数据连接失败，流程终止。");
      return std::nullopt;
    }
  }

  if (options.validate_logic) {
    tracer_core::application::runtime_bridge::LogInfo(
        "[STEP] Step: Validating Business Logic...");
    if (!LogicValidationStage::Execute(session)) {
      return std::nullopt;
    }
  }

  if (options.convert && options.save_processed_output) {
    tracer_core::application::runtime_bridge::LogInfo(
        "[STEP] Step: Saving Validated JSON...");

    auto new_files = processed_data_storage_->WriteProcessedData(
        session.result.processed_data, session.config.output_root);

    session.state.generated_files.insert(session.state.generated_files.end(),
                                         new_files.begin(), new_files.end());

    if (new_files.empty() && !session.result.processed_data.empty()) {
      tracer_core::application::runtime_bridge::LogWarn(
          "[WARN] Data exists but no files were generated (flush failed).");
    } else {
      tracer_core::application::runtime_bridge::LogInfo(
          "[OK] JSON data safely persisted to disk.");
    }
  }

  tracer_core::application::runtime_bridge::LogInfo(
      "\n--- Pipeline Finished Successfully ---");
  return session;
}

}  // namespace tracer::core::application::pipeline
