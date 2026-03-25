#include "application/pipeline/pipeline_workflow.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <format>
#include <fstream>
#include <iomanip>
#include <map>
#include <memory>
#include <optional>
#include <ranges>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "application/pipeline/importer/import_service.hpp"
#include "application/runtime_bridge/logger.hpp"
#include "domain/logic/converter/convert/core/converter_core.hpp"
#include "domain/logic/converter/log_processor.hpp"
#include "domain/model/daily_log.hpp"
#include "domain/ports/diagnostics.hpp"
#include "shared/utils/canonical_text.hpp"
#include "shared/utils/string_utils.hpp"

import tracer.core.application.pipeline.orchestrator;
import tracer.core.application.pipeline.types;
import tracer.core.domain.types.app_options;

namespace tracer::core::application::pipeline {

namespace fs = std::filesystem;
namespace app_ports = tracer_core::application::ports;
namespace runtime_bridge = tracer_core::application::runtime_bridge;
namespace modtext = tracer::core::shared::canonical_text;
namespace modports = tracer_core::domain::ports;
using tracer::core::domain::types::AppOptions;
using tracer::core::shared::string_utils::Trim;

namespace {

#include "application/pipeline/detail/pipeline_workflow_support_impl.inc"

}  // namespace

PipelineWorkflow::PipelineWorkflow(
    fs::path output_root_path, ProcessedDataLoaderPtr processed_data_loader,
    TimeSheetRepositoryPtr time_sheet_repository,
    DatabaseHealthCheckerPtr database_health_checker,
    ConverterConfigProviderPtr converter_config_provider,
    IngestInputProviderPtr ingest_input_provider,
    ProcessedDataStoragePtr processed_data_storage,
    ValidationIssueReporterPtr validation_issue_reporter)
    : output_root_path_(std::move(output_root_path)),
      processed_data_loader_(std::move(processed_data_loader)),
      time_sheet_repository_(std::move(time_sheet_repository)),
      database_health_checker_(std::move(database_health_checker)),
      converter_config_provider_(std::move(converter_config_provider)),
      ingest_input_provider_(std::move(ingest_input_provider)),
      processed_data_storage_(std::move(processed_data_storage)),
      validation_issue_reporter_(std::move(validation_issue_reporter)) {
  if (!processed_data_loader_ || !time_sheet_repository_ ||
      !database_health_checker_ || !converter_config_provider_ ||
      !ingest_input_provider_ || !processed_data_storage_ ||
      !validation_issue_reporter_) {
    throw std::invalid_argument(
        "PipelineWorkflow dependencies must not be null.");
  }
}

PipelineWorkflow::~PipelineWorkflow() = default;

auto PipelineWorkflow::RunConverter(const std::string& input_path,
                                    const AppOptions& options) -> void {
  (void)input_path;
  PipelineOrchestrator pipeline(output_root_path_, converter_config_provider_,
                                ingest_input_provider_, processed_data_storage_,
                                validation_issue_reporter_);
  RunPipelineOrThrow(pipeline, options, "Converter Pipeline Failed.");
}

auto PipelineWorkflow::RunValidateStructure(const std::string& source_path)
    -> void {
  modports::ClearBufferedDiagnostics();
  const AppOptions options = BuildStructureValidationOptions(source_path);
  const ScopedErrorReportWriterOverride disable_error_reports(nullptr);

  PipelineOrchestrator pipeline(output_root_path_, converter_config_provider_,
                                ingest_input_provider_, processed_data_storage_,
                                validation_issue_reporter_);
  RunPipelineOrThrow(pipeline, options, "Validate structure pipeline failed.");
}

auto PipelineWorkflow::RunValidateLogic(const std::string& source_path,
                                        DateCheckMode date_check_mode) -> void {
  modports::ClearBufferedDiagnostics();
  const AppOptions options =
      BuildLogicValidationOptions(source_path, date_check_mode);
  const ScopedErrorReportWriterOverride disable_error_reports(nullptr);

  PipelineOrchestrator pipeline(output_root_path_, converter_config_provider_,
                                ingest_input_provider_, processed_data_storage_,
                                validation_issue_reporter_);
  RunPipelineOrThrow(pipeline, options, "Validate logic pipeline failed.");
}

auto PipelineWorkflow::RunDatabaseImport(const std::string& processed_path_str)
    -> void {
  runtime_bridge::LogInfo("正在解析 JSON 数据...");

  auto load_result = processed_data_loader_->LoadDailyLogs(processed_path_str);
  for (const auto& error : load_result.errors) {
    runtime_bridge::LogError("解析文件失败 " + error.source + ": " +
                             error.message);
  }
  if (load_result.data_by_source.empty()) {
    runtime_bridge::LogWarn("没有有效的 JSON 数据可供导入。");
    return;
  }

  RunDatabaseImportFromMemory(load_result.data_by_source);
}

auto PipelineWorkflow::RunDatabaseImportFromMemory(
    const std::map<std::string, std::vector<DailyLog>>& data_map) -> void {
  runtime_bridge::LogInfo("Task: Memory Import...");
  ImportService service(*time_sheet_repository_);
  ImportStats stats = service.ImportFromMemory(data_map);
  PrintImportStats(stats, "Memory Import");
  ThrowIfImportTaskFailed(stats, "Memory import failed.");
}

auto PipelineWorkflow::RunDatabaseImportFromMemoryReplacingAll(
    const std::map<std::string, std::vector<DailyLog>>& data_map) -> void {
  runtime_bridge::LogInfo("Task: Memory Import (Replace All)...");
  ImportService service(*time_sheet_repository_);
  ImportStats stats =
      service.ImportFromMemory(data_map, std::nullopt, ReplaceAllTarget{});
  PrintImportStats(stats, "Memory Import (replace all)");
  ThrowIfImportTaskFailed(stats, "Memory import (replace all) failed.");
}

auto PipelineWorkflow::RunDatabaseImportFromMemoryReplacingMonth(
    const std::map<std::string, std::vector<DailyLog>>& data_map, int year,
    int month) -> void {
  runtime_bridge::LogInfo("Task: Memory Import (Replace Month)...");
  ImportService service(*time_sheet_repository_);
  ImportStats stats = service.ImportFromMemory(
      data_map, ReplaceMonthTarget{.kYear = year, .kMonth = month});
  PrintImportStats(stats, "Memory Import (Replace Month)");
  ThrowIfImportTaskFailed(stats, "Memory import (replace month) failed.");
}

namespace {

#include "application/pipeline/detail/pipeline_replace_month_support_impl.inc"

}  // namespace

auto PipelineWorkflow::RunIngest(const std::string& source_path,
                                 DateCheckMode date_check_mode,
                                 bool save_processed, IngestMode ingest_mode)
    -> void {
  runtime_bridge::LogInfo("\n--- 启动数据摄入 (Ingest) ---");
  modports::ClearBufferedDiagnostics();

  const auto db_check = database_health_checker_->CheckReady();
  if (!db_check.ok) {
    throw std::runtime_error(db_check.message.empty()
                                 ? "Database readiness check failed."
                                 : db_check.message);
  }

  PipelineOrchestrator pipeline(output_root_path_, converter_config_provider_,
                                ingest_input_provider_, processed_data_storage_,
                                validation_issue_reporter_);
  const AppOptions full_options =
      BuildIngestOptions(source_path, date_check_mode, save_processed);
  auto result_context_opt = pipeline.Run(full_options);

  if (!result_context_opt) {
    runtime_bridge::LogError("\n=== Ingest 执行失败 ===");
    throw std::runtime_error(
        BuildPipelineFailureMessage("Ingestion process failed."));
  }

  auto& context = *result_context_opt;
  runtime_bridge::LogInfo("\n--- 流水线验证通过，准备入库 ---");

  if (ingest_mode == IngestMode::kSingleTxtReplaceMonth) {
    const auto target_month = TryResolveSingleTxtTargetMonth(context);
    if (!target_month.has_value()) {
      throw std::runtime_error(
          "Single TXT replace-month ingest requires exactly one TXT input "
          "with valid headers: yYYYY + mMM.");
    }

    if (!IsSingleMonthConsistent(context.result.processed_data,
                                 target_month->month_key)) {
      throw std::runtime_error(
          "Single TXT replace-month ingest failed: parsed days are not "
          "consistent with header month " +
          target_month->month_key + ".");
    }

    const auto previous_tail = ResolvePreviousTailForReplaceMonth(
        context, *target_month, context.result.processed_data,
        *time_sheet_repository_);
    if (previous_tail.has_value()) {
      LogLinker linker(context.state.converter_config);
      linker.LinkFirstDayWithExternalPreviousEvent(
          context.result.processed_data,
          LogLinker::ExternalPreviousEvent{
              .date = previous_tail->date,
              .end_time = previous_tail->end_time,
          });
    } else if (!context.result.processed_data.empty()) {
      runtime_bridge::LogWarn(
          "[LogLinker] No previous-month tail context found (DB/sibling "
          "TXT). Ingest will proceed without cross-month backfill.");
    }

    RunDatabaseImportFromMemoryReplacingMonth(
        context.result.processed_data, target_month->year, target_month->month);
    runtime_bridge::LogInfo("\n=== Ingest 执行成功（单月替换）===");
    return;
  }

  if (!context.result.processed_data.empty()) {
    RunDatabaseImportFromMemory(context.result.processed_data);
    runtime_bridge::LogInfo("\n=== Ingest 执行成功 ===");
  } else {
    runtime_bridge::LogWarn("\n=== Ingest 完成但无数据产生 ===");
  }
}

auto PipelineWorkflow::RunIngestReplacingAll(const std::string& source_path,
                                             DateCheckMode date_check_mode,
                                             bool save_processed) -> void {
  runtime_bridge::LogInfo("\n--- 启动数据摄入 (Replace All) ---");
  modports::ClearBufferedDiagnostics();

  const auto db_check = database_health_checker_->CheckReady();
  if (!db_check.ok) {
    throw std::runtime_error(db_check.message.empty()
                                 ? "Database readiness check failed."
                                 : db_check.message);
  }

  PipelineOrchestrator pipeline(output_root_path_, converter_config_provider_,
                                ingest_input_provider_, processed_data_storage_,
                                validation_issue_reporter_);
  const AppOptions full_options =
      BuildIngestOptions(source_path, date_check_mode, save_processed);
  auto result_context_opt = pipeline.Run(full_options);

  if (!result_context_opt) {
    runtime_bridge::LogError("\n=== Replace-all ingest 执行失败 ===");
    throw std::runtime_error(
        BuildPipelineFailureMessage("Replace-all ingestion process failed."));
  }

  auto& context = *result_context_opt;
  runtime_bridge::LogInfo("\n--- 流水线验证通过，准备全量替换入库 ---");
  RunDatabaseImportFromMemoryReplacingAll(context.result.processed_data);
  runtime_bridge::LogInfo("\n=== Ingest 执行成功（全量替换）===");
}

}  // namespace tracer::core::application::pipeline
