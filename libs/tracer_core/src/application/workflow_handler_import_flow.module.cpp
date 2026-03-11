module;

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <format>
#include <fstream>
#include <map>
#include <memory>
#include <optional>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "application/ports/i_database_health_checker.hpp"
#include "application/ports/i_time_sheet_repository.hpp"
#include "application/ports/logger.hpp"
#include "domain/types/date_check_mode.hpp"
#include "domain/types/ingest_mode.hpp"

namespace fs = std::filesystem;

struct AppOptions;
struct DailyLog;

namespace tracer_core::application::ports {
class IConverterConfigProvider;
class IDatabaseHealthChecker;
class IIngestInputProvider;
class IProcessedDataLoader;
class IProcessedDataStorage;
class ITimeSheetRepository;
class IValidationIssueReporter;
}  // namespace tracer_core::application::ports

module tracer.core.application.workflow.handler;

import tracer.core.application.workflow.interface;
import tracer.core.application.pipeline.orchestrator;
import tracer.core.application.pipeline.types;
import tracer.core.domain.logic.converter.core;
import tracer.core.domain.logic.converter.log_processor;
import tracer.core.domain.model.daily_log;
import tracer.core.domain.ports.diagnostics;
import tracer.core.domain.types.app_options;
import tracer.core.domain.types.date_check_mode;
import tracer.core.domain.types.ingest_mode;
import tracer.core.shared.ansi_colors;
import tracer.core.shared.string_utils;

namespace app_ports = tracer_core::application::ports;
namespace app_pipeline = tracer::core::application::pipeline;
using tracer::core::shared::string_utils::Trim;
namespace modcolors = tracer::core::shared::ansi_colors;
using tracer::core::domain::modlogic::converter::LogLinker;
using tracer::core::domain::modlogic::converter::LogProcessor;
using tracer::core::domain::model::DailyLog;
using tracer::core::domain::types::AppOptions;
using tracer::core::domain::types::DateCheckMode;
using tracer::core::domain::types::IngestMode;
namespace modports = tracer::core::domain::ports;

using app_pipeline::PipelineOrchestrator;
using app_pipeline::PipelineSession;

namespace workflow_handler_internal {
[[nodiscard]] auto BuildPipelineFailureMessage(std::string_view base_message)
    -> std::string;
}  // namespace workflow_handler_internal

#include "application/internal/workflow_handler_import_namespace.inc"

namespace tracer::core::application::workflow {

auto WorkflowHandler::RunIngest(const std::string& source_path,
                                DateCheckMode date_check_mode,
                                bool save_processed, IngestMode ingest_mode)
    -> void {
  app_ports::LogInfo("\n--- 启动数据摄入 (Ingest) ---");
  modports::ClearBufferedDiagnostics();

  const auto kDbCheck = database_health_checker_->CheckReady();
  if (!kDbCheck.ok) {
    throw std::runtime_error(kDbCheck.message.empty()
                                 ? "Database readiness check failed."
                                 : kDbCheck.message);
  }

  PipelineOrchestrator pipeline(output_root_path_, converter_config_provider_,
                                ingest_input_provider_,
                                processed_data_storage_,
                                validation_issue_reporter_);
  const AppOptions kFullOptions =
      BuildIngestOptions(source_path, date_check_mode, save_processed);
  auto result_context_opt = pipeline.Run(kFullOptions);

  if (!result_context_opt) {
    app_ports::LogError(std::string(modcolors::kRed) +
                        "\n=== Ingest 执行失败 ===" + modcolors::kReset.data());
    throw std::runtime_error(
        workflow_handler_internal::BuildPipelineFailureMessage(
            "Ingestion process failed."));
  }

  auto& context = *result_context_opt;
  app_ports::LogInfo("\n--- 流水线验证通过，准备入库 ---");

  if (ingest_mode == IngestMode::kSingleTxtReplaceMonth) {
    const auto kTargetMonth = TryResolveSingleTxtTargetMonth(context);
    if (!kTargetMonth.has_value()) {
      throw std::runtime_error(
          "Single TXT replace-month ingest requires exactly one TXT input "
          "with valid headers: yYYYY + mMM.");
    }

    if (!IsSingleMonthConsistent(context.result.processed_data,
                                 kTargetMonth->month_key)) {
      throw std::runtime_error(
          "Single TXT replace-month ingest failed: parsed days are not "
          "consistent with header month " +
          kTargetMonth->month_key + ".");
    }

    const auto kPreviousTail = ResolvePreviousTailForReplaceMonth(
        context, *kTargetMonth, context.result.processed_data,
        *time_sheet_repository_);
    if (kPreviousTail.has_value()) {
      LogLinker linker(context.state.converter_config);
      linker.LinkFirstDayWithExternalPreviousEvent(
          context.result.processed_data,
          LogLinker::ExternalPreviousEvent{
              .date = kPreviousTail->date,
              .end_time = kPreviousTail->end_time,
          });
    } else if (!context.result.processed_data.empty()) {
      app_ports::LogWarn(
          "[LogLinker] No previous-month tail context found (DB/sibling "
          "TXT). Ingest will proceed without cross-month backfill.");
    }

    RunDatabaseImportFromMemoryReplacingMonth(
        context.result.processed_data, kTargetMonth->year, kTargetMonth->month);
    app_ports::LogInfo(
        std::string(modcolors::kGreen) +
        "\n=== Ingest 执行成功（单月替换）===" + modcolors::kReset.data());
    return;
  }

  if (!context.result.processed_data.empty()) {
    RunDatabaseImportFromMemory(context.result.processed_data);
    app_ports::LogInfo(std::string(modcolors::kGreen) +
                       "\n=== Ingest 执行成功 ===" + modcolors::kReset.data());
  } else {
    app_ports::LogWarn(
        std::string(modcolors::kYellow) +
        "\n=== Ingest 完成但无数据产生 ===" + modcolors::kReset.data());
  }
}

}  // namespace tracer::core::application::workflow
