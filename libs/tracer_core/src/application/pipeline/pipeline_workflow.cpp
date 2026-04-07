#include "application/pipeline/pipeline_workflow.hpp"

#include <algorithm>
#include <array>
#include <atomic>
#include <cctype>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <iomanip>
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <ranges>
#include <set>
#include <sstream>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "application/pipeline/importer/import_service.hpp"
#include "application/pipeline/detail/pipeline_converter_config_install.hpp"
#include "application/pipeline/detail/pipeline_sha256.hpp"
#include "application/pipeline/detail/pipeline_record_time_order_support.hpp"
#include "application/runtime_bridge/logger.hpp"
#include "domain/logic/converter/convert/core/converter_core.hpp"
#include "domain/logic/converter/log_processor.hpp"
#include "domain/model/daily_log.hpp"
#include "domain/ports/diagnostics.hpp"
#include "shared/utils/canonical_text.hpp"
#include "shared/utils/string_utils.hpp"

import tracer.core.application.pipeline.orchestrator;
import tracer.core.application.pipeline.stages;
import tracer.core.application.pipeline.types;
import tracer.core.domain.types.app_options;

namespace tracer::core::application::pipeline {

namespace fs = std::filesystem;
namespace app_ports = tracer_core::application::ports;
namespace runtime_bridge = tracer_core::application::runtime_bridge;
namespace modtext = tracer::core::shared::canonical_text;
namespace modports = tracer_core::domain::ports;
namespace pipeline_detail = tracer::core::application::pipeline::detail;
using tracer::core::domain::types::AppOptions;
using tracer::core::shared::string_utils::Trim;
using tracer_core::application::dto::IngestInputModel;
using tracer_core::core::dto::IngestSyncStatusEntry;
using tracer_core::core::dto::IngestSyncStatusOutput;
using tracer_core::core::dto::IngestSyncStatusRequest;
using tracer_core::core::dto::RecordActivityAtomicallyRequest;
using tracer_core::core::dto::RecordActivityAtomicallyResponse;

namespace {

#include "application/pipeline/detail/pipeline_workflow_support_impl.inc"
#include "application/pipeline/detail/pipeline_replace_month_support_impl.inc"
#include "application/pipeline/detail/pipeline_record_alias_text_support_impl.inc"
#include "application/pipeline/detail/pipeline_record_atomic_support_impl.inc"

[[nodiscard]] auto BuildCanonicalMonthRelativePath(
    const SingleTxtTargetMonth& month) -> std::string {
  return std::format("{0:04d}/{0:04d}-{1:02d}.txt", month.year, month.month);
}

[[nodiscard]] auto TryBuildIngestSyncEntry(const IngestInputModel& input,
                                           const std::int64_t kIngestedAtMs)
    -> std::optional<IngestSyncStatusEntry> {
  const auto kCanonical = modtext::Canonicalize(
      input.content, input.source_label.empty() ? input.source_id
                                                : input.source_label);
  if (!kCanonical.ok) {
    runtime_bridge::LogWarn("Skipping ingest sync snapshot due to invalid TXT: " +
                            kCanonical.error_message);
    return std::nullopt;
  }

  const auto kTargetMonth =
      TryParseSingleTxtTargetMonthFromContent(kCanonical.text);
  if (!kTargetMonth.has_value()) {
    runtime_bridge::LogWarn(
        "Skipping ingest sync snapshot because TXT month header is missing: " +
        (input.source_label.empty() ? input.source_id : input.source_label));
    return std::nullopt;
  }

  return IngestSyncStatusEntry{
      .month_key = kTargetMonth->month_key,
      .txt_relative_path = BuildCanonicalMonthRelativePath(*kTargetMonth),
      .txt_content_hash_sha256 = pipeline_detail::ComputeSha256Hex(
          kCanonical.text),
      .ingested_at_unix_ms = kIngestedAtMs,
  };
}

[[nodiscard]] auto BuildIngestSyncSnapshot(const PipelineSession& context)
    -> std::vector<IngestSyncStatusEntry> {
  std::map<std::string, IngestSyncStatusEntry> unique_entries;
  std::set<std::string> duplicate_months;
  const std::int64_t kIngestedAtMs = pipeline_detail::CurrentUnixMillis();

  for (const auto& input : context.state.ingest_inputs) {
    const auto kEntry = TryBuildIngestSyncEntry(input, kIngestedAtMs);
    if (!kEntry.has_value()) {
      continue;
    }

    if (duplicate_months.contains(kEntry->month_key)) {
      continue;
    }

    const auto kInsertResult =
        unique_entries.emplace(kEntry->month_key, *kEntry);
    if (!kInsertResult.second) {
      duplicate_months.insert(kEntry->month_key);
      unique_entries.erase(kEntry->month_key);
      runtime_bridge::LogWarn(
          "Duplicate TXT month detected during ingest sync snapshot: " +
          kEntry->month_key +
          ". Sync row will be omitted for this month.");
    }
  }

  std::vector<IngestSyncStatusEntry> snapshot;
  snapshot.reserve(unique_entries.size());
  for (auto& [month_key, entry] : unique_entries) {
    (void)month_key;
    snapshot.push_back(std::move(entry));
  }
  return snapshot;
}

auto PersistIngestSyncSnapshot(const PipelineSession& context,
                               app_ports::ITimeSheetRepository& repository)
    -> void {
  repository.ReplaceIngestSyncStatuses(BuildIngestSyncSnapshot(context));
}

auto PersistSingleIngestSyncEntry(const PipelineSession& context,
                                  app_ports::ITimeSheetRepository& repository)
    -> void {
  if (context.state.ingest_inputs.size() != 1U) {
    throw std::runtime_error(
        "Single TXT ingest sync snapshot requires exactly one input.");
  }

  const auto kEntry =
      TryBuildIngestSyncEntry(context.state.ingest_inputs.front(),
                              pipeline_detail::CurrentUnixMillis());
  if (!kEntry.has_value()) {
    throw std::runtime_error(
        "Single TXT ingest sync snapshot requires valid yYYYY + mMM headers.");
  }

  repository.UpsertIngestSyncStatus(*kEntry);
}

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
  const AppOptions kOptions = BuildStructureValidationOptions(source_path);
  const ScopedErrorReportWriterOverride kDisableErrorReports(nullptr);

  PipelineOrchestrator pipeline(output_root_path_, converter_config_provider_,
                                ingest_input_provider_, processed_data_storage_,
                                validation_issue_reporter_);
  RunPipelineOrThrow(pipeline, kOptions, "Validate structure pipeline failed.");
}

auto PipelineWorkflow::RunValidateLogic(const std::string& source_path,
                                        DateCheckMode date_check_mode) -> void {
  modports::ClearBufferedDiagnostics();
  const AppOptions kOptions =
      BuildLogicValidationOptions(source_path, date_check_mode);
  const ScopedErrorReportWriterOverride kDisableErrorReports(nullptr);

  PipelineOrchestrator pipeline(output_root_path_, converter_config_provider_,
                                ingest_input_provider_, processed_data_storage_,
                                validation_issue_reporter_);
  RunPipelineOrThrow(pipeline, kOptions, "Validate logic pipeline failed.");
}

auto PipelineWorkflow::RunRecordActivityAtomically(
    const RecordActivityAtomicallyRequest& request)
    -> RecordActivityAtomicallyResponse {
  // This is orchestration only: delegate atomic TXT candidate build/validate/ingest+rollback
  // to dedicated record helpers, while keeping workflow-owned RunIngest invocation here.
  return RunRecordActivityAtomicallySupport(
      request, output_root_path_, *converter_config_provider_,
      validation_issue_reporter_,
      [this](const std::string& source_path,
             const DateCheckMode kDateCheckMode) -> void {
        RunIngest(source_path, kDateCheckMode, false,
                  IngestMode::kSingleTxtReplaceMonth);
      });
}

auto PipelineWorkflow::InstallActiveConverterConfig(
    const ActiveConverterConfigInstallRequest& request) -> void {
  const auto kSourcePaths =
      pipeline_detail::ResolveConverterConfigPathSet(
          request.source_main_config_path);
  const auto kTargetPaths =
      pipeline_detail::ResolveConverterConfigPathSet(
          request.target_main_config_path);

  pipeline_detail::EnsureConverterConfigSourceExists(
      kSourcePaths.main_config_path, "Converter main config");
  pipeline_detail::EnsureConverterConfigSourceExists(
      kSourcePaths.alias_mapping_path, "Alias mapping config");
  pipeline_detail::EnsureConverterConfigSourceExists(
      kSourcePaths.duration_rules_path, "Duration rules config");

  pipeline_detail::CopyConverterConfigFile(
      kSourcePaths.main_config_path, kTargetPaths.main_config_path,
      "converter main config");
  pipeline_detail::CopyConverterConfigFile(
      kSourcePaths.alias_mapping_path, kTargetPaths.alias_mapping_path,
      "alias mapping config");
  pipeline_detail::CopyConverterConfigFile(
      kSourcePaths.duration_rules_path, kTargetPaths.duration_rules_path,
      "duration rules config");
  pipeline_detail::RemoveConverterAliasDirectory(
      kTargetPaths.alias_mapping_path.parent_path());
  pipeline_detail::CopyConverterAliasDirectory(
      kSourcePaths.alias_directory_path, kTargetPaths.alias_directory_path);
  converter_config_provider_->InvalidateCache();
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

auto PipelineWorkflow::RunIngest(const std::string& source_path,
                                 DateCheckMode date_check_mode,
                                 bool save_processed, IngestMode ingest_mode)
    -> void {
  runtime_bridge::LogInfo("\n--- 启动数据摄入 (Ingest) ---");
  modports::ClearBufferedDiagnostics();

  const auto kDbCheck = database_health_checker_->CheckReady();
  if (!kDbCheck.ok) {
    throw std::runtime_error(kDbCheck.message.empty()
                                 ? "Database readiness check failed."
                                 : kDbCheck.message);
  }

  PipelineOrchestrator pipeline(output_root_path_, converter_config_provider_,
                                ingest_input_provider_, processed_data_storage_,
                                validation_issue_reporter_);
  const AppOptions kFullOptions =
      BuildIngestOptions(source_path, date_check_mode, save_processed);
  auto result_context_opt = pipeline.Run(kFullOptions);

  if (!result_context_opt) {
    runtime_bridge::LogError("\n=== Ingest 执行失败 ===");
    throw std::runtime_error(
        BuildPipelineFailureMessage("Ingestion process failed."));
  }

  auto& context = *result_context_opt;
  runtime_bridge::LogInfo("\n--- 流水线验证通过，准备入库 ---");

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
      runtime_bridge::LogWarn(
          "[LogLinker] No previous-month tail context found (DB/sibling "
          "TXT). Ingest will proceed without cross-month backfill.");
    }

    RunDatabaseImportFromMemoryReplacingMonth(
        context.result.processed_data, kTargetMonth->year,
        kTargetMonth->month);
    PersistSingleIngestSyncEntry(context, *time_sheet_repository_);
    runtime_bridge::LogInfo("\n=== Ingest 执行成功（单月替换）===");
    return;
  }

  if (!context.result.processed_data.empty()) {
    RunDatabaseImportFromMemory(context.result.processed_data);
    PersistIngestSyncSnapshot(context, *time_sheet_repository_);
    runtime_bridge::LogInfo("\n=== Ingest 执行成功 ===");
  } else {
    PersistIngestSyncSnapshot(context, *time_sheet_repository_);
    runtime_bridge::LogWarn("\n=== Ingest 完成但无数据产生 ===");
  }
}

auto PipelineWorkflow::RunIngestSyncStatusQuery(
    const IngestSyncStatusRequest& request) -> IngestSyncStatusOutput {
  return time_sheet_repository_->ListIngestSyncStatuses(request);
}

auto PipelineWorkflow::ClearIngestSyncStatus() -> void {
  time_sheet_repository_->ClearIngestSyncStatus();
}

auto PipelineWorkflow::RunIngestReplacingAll(const std::string& source_path,
                                             DateCheckMode date_check_mode,
                                             bool save_processed) -> void {
  runtime_bridge::LogInfo("\n--- 启动数据摄入 (Replace All) ---");
  modports::ClearBufferedDiagnostics();

  const auto kDbCheck = database_health_checker_->CheckReady();
  if (!kDbCheck.ok) {
    throw std::runtime_error(kDbCheck.message.empty()
                                 ? "Database readiness check failed."
                                 : kDbCheck.message);
  }

  PipelineOrchestrator pipeline(output_root_path_, converter_config_provider_,
                                ingest_input_provider_, processed_data_storage_,
                                validation_issue_reporter_);
  const AppOptions kFullOptions =
      BuildIngestOptions(source_path, date_check_mode, save_processed);
  auto result_context_opt = pipeline.Run(kFullOptions);

  if (!result_context_opt) {
    runtime_bridge::LogError("\n=== Replace-all ingest 执行失败 ===");
    throw std::runtime_error(
        BuildPipelineFailureMessage("Replace-all ingestion process failed."));
  }

  auto& context = *result_context_opt;
  runtime_bridge::LogInfo("\n--- 流水线验证通过，准备全量替换入库 ---");
  RunDatabaseImportFromMemoryReplacingAll(context.result.processed_data);
  PersistIngestSyncSnapshot(context, *time_sheet_repository_);
  runtime_bridge::LogInfo("\n=== Ingest 执行成功（全量替换）===");
}

}  // namespace tracer::core::application::pipeline

