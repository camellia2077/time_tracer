#include "application/pipeline/pipeline_workflow.hpp"
#include "application/pipeline/pipeline_ingest_service.hpp"

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
#include <tuple>
#include <utility>
#include <unordered_map>
#include <vector>

#include "application/pipeline/txt_day_block_support.hpp"
#include "application/activity_name_converter.hpp"
#include "application/pipeline/detail/pipeline_converter_config_install.hpp"
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
using tracer_core::core::dto::ConvertTxtActivityNamesRequest;
using tracer_core::core::dto::ConvertTxtActivityNamesResponse;
using tracer_core::core::dto::DefaultTxtDayMarkerRequest;
using tracer_core::core::dto::DefaultTxtDayMarkerResponse;
using tracer_core::core::dto::IngestSyncStatusEntry;
using tracer_core::core::dto::IngestSyncStatusOutput;
using tracer_core::core::dto::IngestSyncStatusRequest;
using tracer_core::core::dto::RecordActivityAtomicallyRequest;
using tracer_core::core::dto::RecordActivityAtomicallyResponse;
using tracer_core::core::dto::ReplaceTxtAliasActivityNamesRequest;
using tracer_core::core::dto::ReplaceTxtAliasActivityNamesResponse;
using tracer_core::core::dto::ReplaceTxtCanonicalActivityNamesRequest;
using tracer_core::core::dto::ReplaceTxtCanonicalActivityNamesResponse;
using tracer_core::core::dto::ReplaceTxtDayBlockRequest;
using tracer_core::core::dto::ReplaceTxtDayBlockResponse;
using tracer_core::core::dto::ResolveTxtDayBlockRequest;
using tracer_core::core::dto::ResolveTxtDayBlockResponse;
using tracer_core::core::dto::UpdateActivityRemarkAtomicallyRequest;
using tracer_core::core::dto::UpdateActivityRemarkAtomicallyResponse;
using tracer_core::core::dto::UpdateDayRemarkAtomicallyRequest;
using tracer_core::core::dto::UpdateDayRemarkAtomicallyResponse;

namespace {

#include "application/pipeline/detail/pipeline_workflow_support_impl.inc"
#include "application/pipeline/detail/pipeline_replace_month_support_impl.inc"
#include "application/pipeline/detail/pipeline_record_alias_text_support_impl.inc"
#include "application/pipeline/detail/pipeline_record_atomic_support_impl.inc"
#include "application/pipeline/detail/pipeline_update_remark_support_impl.inc"
#include "application/pipeline/detail/pipeline_update_day_remark_support_impl.inc"

}  // namespace

PipelineWorkflow::PipelineWorkflow(
    fs::path output_root_path, ProcessedDataLoaderPtr processed_data_loader,
    TimeSheetWriteRepositoryPtr time_sheet_write_repository,
    IngestRuntimeRepositoryPtr ingest_runtime_repository,
    DatabaseHealthCheckerPtr database_health_checker,
    ConverterConfigProviderPtr converter_config_provider,
    IngestInputProviderPtr ingest_input_provider,
    ProcessedDataStoragePtr processed_data_storage,
    ValidationIssueReporterPtr validation_issue_reporter)
    : output_root_path_(std::move(output_root_path)),
      processed_data_loader_(std::move(processed_data_loader)),
      time_sheet_write_repository_(std::move(time_sheet_write_repository)),
      ingest_runtime_repository_(std::move(ingest_runtime_repository)),
      database_health_checker_(std::move(database_health_checker)),
      converter_config_provider_(std::move(converter_config_provider)),
      ingest_input_provider_(std::move(ingest_input_provider)),
      processed_data_storage_(std::move(processed_data_storage)),
      validation_issue_reporter_(std::move(validation_issue_reporter)) {
  if (!processed_data_loader_ || !time_sheet_write_repository_ ||
      !ingest_runtime_repository_ || !database_health_checker_ ||
      !converter_config_provider_ || !ingest_input_provider_ ||
      !processed_data_storage_ || !validation_issue_reporter_) {
    throw std::invalid_argument(
        "PipelineWorkflow dependencies must not be null.");
  }

  ingest_service_ = std::make_unique<PipelineIngestService>(
      output_root_path_, processed_data_loader_, time_sheet_write_repository_,
      ingest_runtime_repository_, database_health_checker_,
      converter_config_provider_, ingest_input_provider_,
      processed_data_storage_, validation_issue_reporter_);
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
  // This is orchestration only: delegate atomic TXT candidate
  // build/validate/ingest+rollback to dedicated record helpers, while
  // delegating ingest invocation to the dedicated PipelineIngestService.
  return RunRecordActivityAtomicallySupport(
      request, output_root_path_, *converter_config_provider_,
      validation_issue_reporter_,
      [this](const std::string& source_path,
             const DateCheckMode kDateCheckMode) -> void {
        RunIngest(source_path, kDateCheckMode, false,
                  IngestMode::kSingleTxtReplaceMonth);
      });
}

auto PipelineWorkflow::RunDefaultTxtDayMarker(
    const DefaultTxtDayMarkerRequest& request) -> DefaultTxtDayMarkerResponse {
  return txt_day_block::DefaultDayMarker(request);
}

auto PipelineWorkflow::RunResolveTxtDayBlock(
    const ResolveTxtDayBlockRequest& request) -> ResolveTxtDayBlockResponse {
  return txt_day_block::ResolveDayBlock(request);
}

auto PipelineWorkflow::RunReplaceTxtDayBlock(
    const ReplaceTxtDayBlockRequest& request) -> ReplaceTxtDayBlockResponse {
  return txt_day_block::ReplaceDayBlock(request);
}

auto PipelineWorkflow::RunUpdateActivityRemarkAtomically(
    const UpdateActivityRemarkAtomicallyRequest& request)
    -> UpdateActivityRemarkAtomicallyResponse {
  return RunUpdateActivityRemarkAtomicallySupport(
      request, output_root_path_, converter_config_provider_,
      validation_issue_reporter_,
      [this](const std::string& source_path,
             const DateCheckMode kDateCheckMode) -> void {
        RunIngest(source_path, kDateCheckMode, false,
                  IngestMode::kSingleTxtReplaceMonth);
      });
}

auto PipelineWorkflow::RunUpdateDayRemarkAtomically(
    const UpdateDayRemarkAtomicallyRequest& request)
    -> UpdateDayRemarkAtomicallyResponse {
  return RunUpdateDayRemarkAtomicallySupport(
      request, output_root_path_, converter_config_provider_,
      validation_issue_reporter_,
      [this](const std::string& source_path,
             const DateCheckMode kDateCheckMode) -> void {
        RunIngest(source_path, kDateCheckMode, false,
                  IngestMode::kSingleTxtReplaceMonth);
      });
}

auto PipelineWorkflow::RunConvertTxtActivityNames(
    const ConvertTxtActivityNamesRequest& request)
    -> ConvertTxtActivityNamesResponse {
  ActivityNameMappingDirection direction;
  if (request.direction == "alias_to_canonical") {
    direction = ActivityNameMappingDirection::kAliasToCanonical;
  } else if (request.direction == "canonical_to_alias") {
    direction = ActivityNameMappingDirection::kCanonicalToAlias;
  } else {
    throw std::invalid_argument(
        "direction must be alias_to_canonical or canonical_to_alias.");
  }

  const ActivityNameTextConverter kConverter(
      converter_config_provider_->LoadConverterConfig());
  return {
      .ok = true,
      .converted_content = kConverter.ConvertText(request.content, direction),
      .error_message = ""};
}

auto PipelineWorkflow::RunReplaceTxtCanonicalActivityNames(
    const ReplaceTxtCanonicalActivityNamesRequest& request)
    -> ReplaceTxtCanonicalActivityNamesResponse {
  std::unordered_map<std::string, std::string> replacements;
  replacements.reserve(request.replacements.size());
  for (const auto& replacement : request.replacements) {
    if (replacement.old_canonical.empty() ||
        replacement.new_canonical.empty()) {
      throw std::invalid_argument(
          "canonical replacement names must not be empty.");
    }
    const auto [_, inserted] = replacements.emplace(replacement.old_canonical,
                                                    replacement.new_canonical);
    if (!inserted) {
      throw std::invalid_argument(
          "canonical replacement source must be unique: " +
          replacement.old_canonical);
    }
  }
  const ActivityNameTextConverter kConverter(
      converter_config_provider_->LoadConverterConfig());
  return {.ok = true,
          .updated_content =
              kConverter.ReplaceCanonicalNames(request.content, replacements),
          .error_message = ""};
}

auto PipelineWorkflow::RunReplaceTxtAliasActivityNames(
    const ReplaceTxtAliasActivityNamesRequest& request)
    -> ReplaceTxtAliasActivityNamesResponse {
  std::unordered_map<std::string, std::string> replacements;
  replacements.reserve(request.replacements.size());
  for (const auto& replacement : request.replacements) {
    if (replacement.old_alias.empty() || replacement.new_alias.empty()) {
      throw std::invalid_argument("alias replacement names must not be empty.");
    }
    const auto [_, inserted] =
        replacements.emplace(replacement.old_alias, replacement.new_alias);
    if (!inserted) {
      throw std::invalid_argument("alias replacement source must be unique: " +
                                  replacement.old_alias);
    }
  }
  const ActivityNameTextConverter kConverter(
      converter_config_provider_->LoadConverterConfig());
  return {.ok = true,
          .updated_content =
              kConverter.ReplaceAliasNames(request.content, replacements),
          .error_message = ""};
}

auto PipelineWorkflow::InstallActiveConverterConfig(
    const ActiveConverterConfigInstallRequest& request) -> void {
  const auto kSourcePaths = pipeline_detail::ResolveConverterConfigPathSet(
      request.source_main_config_path);
  const auto kTargetPaths = pipeline_detail::ResolveConverterConfigPathSet(
      request.target_main_config_path);

  pipeline_detail::EnsureConverterConfigSourceExists(
      kSourcePaths.main_config_path, "Converter main config");

  pipeline_detail::CopyConverterConfigFile(kSourcePaths.main_config_path,
                                           kTargetPaths.main_config_path,
                                           "converter main config");
  pipeline_detail::RemoveConverterAliasDirectory(
      kTargetPaths.main_config_path.parent_path().parent_path());
  pipeline_detail::CopyConverterAliasDirectory(
      kSourcePaths.alias_directory_path, kTargetPaths.alias_directory_path);
  converter_config_provider_->InvalidateCache();
}

auto PipelineWorkflow::RunDatabaseImport(const std::string& processed_path_str)
    -> void {
  ingest_service_->RunDatabaseImport(processed_path_str);
}

auto PipelineWorkflow::RunDatabaseImportFromMemory(
    const std::map<std::string, std::vector<DailyLog>>& data_map) -> void {
  ingest_service_->RunDatabaseImportFromMemory(data_map);
}

auto PipelineWorkflow::RunDatabaseImportFromMemoryReplacingAll(
    const std::map<std::string, std::vector<DailyLog>>& data_map) -> void {
  ingest_service_->RunDatabaseImportFromMemoryReplacingAll(data_map);
}

auto PipelineWorkflow::RunDatabaseImportFromMemoryReplacingMonth(
    const std::map<std::string, std::vector<DailyLog>>& data_map, int year,
    int month) -> void {
  ingest_service_->RunDatabaseImportFromMemoryReplacingMonth(data_map, year,
                                                             month);
}

auto PipelineWorkflow::RunIngest(const std::string& source_path,
                                 DateCheckMode date_check_mode,
                                 bool save_processed, IngestMode ingest_mode)
    -> void {
  ingest_service_->RunIngest(source_path, date_check_mode, save_processed,
                             ingest_mode);
}

auto PipelineWorkflow::RunIngestSyncStatusQuery(
    const IngestSyncStatusRequest& request) -> IngestSyncStatusOutput {
  return ingest_service_->RunIngestSyncStatusQuery(request);
}

auto PipelineWorkflow::ClearIngestSyncStatus() -> void {
  ingest_service_->ClearIngestSyncStatus();
}

auto PipelineWorkflow::RunIngestReplacingAll(const std::string& source_path,
                                             DateCheckMode date_check_mode,
                                             bool save_processed) -> void {
  ingest_service_->RunIngestReplacingAll(source_path, date_check_mode,
                                         save_processed);
}

}  // namespace tracer::core::application::pipeline
