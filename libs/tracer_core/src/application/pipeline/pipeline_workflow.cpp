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

#include "application/importer/import_service.hpp"
#include "application/ports/logger.hpp"
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
namespace modtext = tracer::core::shared::canonical_text;
namespace modports = tracer::core::domain::ports;
using tracer::core::shared::string_utils::Trim;
using tracer::core::domain::types::AppOptions;

namespace {

class ScopedErrorReportWriterOverride final {
 public:
  explicit ScopedErrorReportWriterOverride(
      std::shared_ptr<modports::IErrorReportWriter> writer)
      : previous_writer_(modports::GetErrorReportWriter()) {
    modports::SetErrorReportWriter(std::move(writer));
  }

  ScopedErrorReportWriterOverride(const ScopedErrorReportWriterOverride&) =
      delete;
  auto operator=(const ScopedErrorReportWriterOverride&)
      -> ScopedErrorReportWriterOverride& = delete;

  ~ScopedErrorReportWriterOverride() {
    modports::SetErrorReportWriter(previous_writer_);
  }

 private:
  std::shared_ptr<modports::IErrorReportWriter> previous_writer_;
};

[[nodiscard]] auto BuildPipelineFailureMessage(std::string_view base_message)
    -> std::string {
  std::string message(base_message);
  const std::string report_destination =
      modports::GetErrorReportDestinationLabel();
  const std::string diagnostic_summary = modports::GetBufferedDiagnosticsSummary(
      modports::DiagnosticSeverity::kError, 8);
  if (!report_destination.empty() && report_destination != "disabled") {
    message += "\nFull error report: " + report_destination;
  }
  if (!diagnostic_summary.empty()) {
    message += "\nRecent diagnostics:\n" + diagnostic_summary;
  }
  return message;
}

auto ThrowIfImportTaskFailed(const ImportStats& stats,
                             std::string_view default_message) -> void {
  if (stats.db_open_success && stats.transaction_success) {
    return;
  }

  throw std::runtime_error(stats.error_message.empty()
                               ? std::string(default_message)
                               : stats.error_message);
}

[[nodiscard]] auto BuildStructureValidationOptions(
    const std::string& source_path) -> AppOptions {
  AppOptions options;
  options.input_path = source_path;
  options.validate_structure = true;
  options.convert = false;
  options.validate_logic = false;
  options.save_processed_output = false;
  options.date_check_mode = DateCheckMode::kNone;
  return options;
}

[[nodiscard]] auto BuildLogicValidationOptions(const std::string& source_path,
                                               DateCheckMode date_check_mode)
    -> AppOptions {
  AppOptions options;
  options.input_path = source_path;
  options.validate_structure = false;
  options.convert = true;
  options.validate_logic = true;
  options.run_structure_validation_before_conversion = true;
  options.save_processed_output = false;
  options.date_check_mode = date_check_mode;
  return options;
}

[[nodiscard]] auto BuildIngestOptions(const std::string& source_path,
                                      DateCheckMode date_check_mode,
                                      bool save_processed) -> AppOptions {
  AppOptions options;
  options.input_path = source_path;
  options.validate_structure = true;
  options.convert = true;
  options.validate_logic = true;
  options.date_check_mode = date_check_mode;
  options.save_processed_output = save_processed;
  return options;
}

auto RunPipelineOrThrow(PipelineOrchestrator& pipeline,
                        const AppOptions& options,
                        std::string_view failure_message) -> void {
  if (!pipeline.Run(options)) {
    throw std::runtime_error(BuildPipelineFailureMessage(failure_message));
  }
}

auto PrintImportStats(const ImportStats& stats, std::string_view title)
    -> void {
  std::ostringstream stream;
  stream << "\n--- " << title << " Report ---";
  app_ports::LogInfo(stream.str());

  if (!stats.db_open_success) {
    app_ports::LogError(
        "[Fatal] DB Error: " +
        (stats.error_message.empty() ? "Unknown" : stats.error_message));
    return;
  }

  if (!stats.transaction_success) {
    app_ports::LogError("[Fatal] Transaction Failed: " + stats.error_message);
    return;
  }

  const bool has_skipped =
      (stats.skipped_days > 0U) || (stats.skipped_records > 0U);
  const bool has_legacy_failed = !stats.failed_files.empty();

  if (!has_legacy_failed && !has_skipped) {
    app_ports::LogInfo("[Success] Processed " +
                       std::to_string(stats.successful_files) + " items.");
  } else {
    std::ostringstream summary;
    summary << "[Partial] Days=" << stats.successful_days << "/"
            << stats.total_days << ", Records=" << stats.successful_records
            << "/" << stats.total_records;
    if (has_legacy_failed) {
      summary << ", LegacyFailedFiles=" << stats.failed_files.size();
    }
    app_ports::LogWarn(summary.str());

    for (const auto& failed_file : stats.failed_files) {
      app_ports::LogError("  Failed: " + failed_file);
    }
  }

  std::ostringstream breakdown;
  breakdown << "Breakdown: days(total=" << stats.total_days
            << ", success=" << stats.successful_days
            << ", skipped=" << stats.skipped_days << "), records(total="
            << stats.total_records << ", success=" << stats.successful_records
            << ", skipped=" << stats.skipped_records << ")";
  app_ports::LogInfo(breakdown.str());

  if (!stats.reason_buckets.empty()) {
    app_ports::LogInfo("Reason Buckets:");
    for (const auto& [reason, count] : stats.reason_buckets) {
      app_ports::LogInfo("  - " + reason + ": " + std::to_string(count));
    }
  }

  const double total_time =
      stats.parsing_duration_s + stats.db_insertion_duration_s;

  std::ostringstream timing;
  timing << std::fixed << std::setprecision(3)
         << "Timing: Parse=" << stats.parsing_duration_s
         << "s, Insert=" << stats.db_insertion_duration_s
         << "s, Total=" << total_time << "s";
  app_ports::LogInfo(timing.str());

  if (stats.replaced_month.has_value()) {
    app_ports::LogInfo("Replace scope: month=" + *stats.replaced_month);
  }
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
  PipelineOrchestrator pipeline(output_root_path_, converter_config_provider_,
                                ingest_input_provider_, processed_data_storage_,
                                validation_issue_reporter_);
  if (!pipeline.Run(options)) {
    throw std::runtime_error("Converter Pipeline Failed.");
  }
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
  app_ports::LogInfo("正在解析 JSON 数据...");

  auto load_result = processed_data_loader_->LoadDailyLogs(processed_path_str);
  for (const auto& error : load_result.errors) {
    app_ports::LogError("解析文件失败 " + error.source + ": " + error.message);
  }
  if (load_result.data_by_source.empty()) {
    app_ports::LogWarn("没有有效的 JSON 数据可供导入。");
    return;
  }

  RunDatabaseImportFromMemory(load_result.data_by_source);
}

auto PipelineWorkflow::RunDatabaseImportFromMemory(
    const std::map<std::string, std::vector<DailyLog>>& data_map) -> void {
  app_ports::LogInfo("Task: Memory Import...");
  ImportService service(*time_sheet_repository_);
  ImportStats stats = service.ImportFromMemory(data_map);
  PrintImportStats(stats, "Memory Import");
  ThrowIfImportTaskFailed(stats, "Memory import failed.");
}

auto PipelineWorkflow::RunDatabaseImportFromMemoryReplacingAll(
    const std::map<std::string, std::vector<DailyLog>>& data_map) -> void {
  app_ports::LogInfo("Task: Memory Import (Replace All)...");
  ImportService service(*time_sheet_repository_);
  ImportStats stats =
      service.ImportFromMemory(data_map, std::nullopt, ReplaceAllTarget{});
  PrintImportStats(stats, "Memory Import (replace all)");
  ThrowIfImportTaskFailed(stats, "Memory import (replace all) failed.");
}

auto PipelineWorkflow::RunDatabaseImportFromMemoryReplacingMonth(
    const std::map<std::string, std::vector<DailyLog>>& data_map, int year,
    int month) -> void {
  app_ports::LogInfo("Task: Memory Import (Replace Month)...");
  ImportService service(*time_sheet_repository_);
  ImportStats stats = service.ImportFromMemory(
      data_map, ReplaceMonthTarget{.kYear = year, .kMonth = month});
  PrintImportStats(stats, "Memory Import (Replace Month)");
  ThrowIfImportTaskFailed(stats, "Memory import (replace month) failed.");
}

namespace {

struct SingleTxtTargetMonth {
  int year = 0;
  int month = 0;
  std::string month_key;
  std::string month_start_date;
};

struct PreviousMonthInfo {
  int year = 0;
  int month = 0;
  std::string month_key;
};

struct PreviousMonthFileCandidate {
  fs::path path;
  std::string content;
};

struct ActivityTailCandidate {
  std::string date;
  std::string end_time;
  long long end_timestamp = 0;
};

[[nodiscard]] auto ToLowerAsciiCopy(std::string value) -> std::string {
  for (char& ch : value) {
    ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
  }
  return value;
}

[[nodiscard]] auto BuildMonthKey(const int year, const int month)
    -> std::string {
  return std::format("{:04d}-{:02d}", year, month);
}

[[nodiscard]] auto TryBuildPreviousMonthInfo(const int year, const int month)
    -> std::optional<PreviousMonthInfo> {
  if (month < 1 || month > 12) {
    return std::nullopt;
  }

  int previous_year = year;
  int previous_month = month - 1;
  if (previous_month == 0) {
    previous_month = 12;
    --previous_year;
  }

  return PreviousMonthInfo{
      .year = previous_year,
      .month = previous_month,
      .month_key = BuildMonthKey(previous_year, previous_month),
  };
}

[[nodiscard]] auto TryParseSingleTxtTargetMonthFromContent(
    std::string_view content) -> std::optional<SingleTxtTargetMonth> {
  constexpr std::size_t kYearLineLength = 5;
  constexpr int kMaxMonth = 12;
  std::stringstream stream{std::string(content)};
  std::string line;
  std::optional<int> year;
  std::optional<int> month;

  while (std::getline(stream, line)) {
    const std::string trimmed = Trim(line);
    if (trimmed.empty()) {
      continue;
    }

    if (!year.has_value() && trimmed.size() == kYearLineLength &&
        trimmed[0] == 'y') {
      try {
        year = std::stoi(trimmed.substr(1));
      } catch (const std::exception&) {
        return std::nullopt;
      }
      continue;
    }

    if (year.has_value() && !month.has_value() && trimmed.size() == 3 &&
        trimmed[0] == 'm') {
      try {
        const int value = std::stoi(trimmed.substr(1));
        if (value < 1 || value > kMaxMonth) {
          return std::nullopt;
        }
        month = value;
      } catch (const std::exception&) {
        return std::nullopt;
      }
      continue;
    }
  }

  if (!year.has_value() || !month.has_value()) {
    return std::nullopt;
  }

  const std::string month_key = BuildMonthKey(*year, *month);
  return SingleTxtTargetMonth{
      .year = *year,
      .month = *month,
      .month_key = month_key,
      .month_start_date = month_key + "-01",
  };
}

[[nodiscard]] auto TryResolveSingleTxtTargetMonth(
    const PipelineSession& context) -> std::optional<SingleTxtTargetMonth> {
  if (context.state.ingest_inputs.size() != 1U) {
    return std::nullopt;
  }
  return TryParseSingleTxtTargetMonthFromContent(
      context.state.ingest_inputs.front().content);
}

[[nodiscard]] auto IsSingleMonthConsistent(
    const std::map<std::string, std::vector<DailyLog>>& processed_data,
    std::string_view month_key) -> bool {
  constexpr size_t kMonthKeyLength = 7;
  for (const auto& [key, days] : processed_data) {
    if (key != month_key) {
      return false;
    }
    for (const auto& day : days) {
      if (day.date.size() < kMonthKeyLength ||
          day.date.substr(0, kMonthKeyLength) != month_key) {
        return false;
      }
    }
  }
  return true;
}

[[nodiscard]] auto TryReadTextFile(const fs::path& path)
    -> std::optional<std::string> {
  std::ifstream input(path, std::ios::in | std::ios::binary);
  if (!input.is_open()) {
    return std::nullopt;
  }

  std::ostringstream buffer;
  buffer << input.rdbuf();
  if (input.bad()) {
    return std::nullopt;
  }

  const std::string raw_text = buffer.str();
  const auto canonical = modtext::Canonicalize(raw_text, path.string());
  if (!canonical.ok) {
    app_ports::LogWarn("Skipping sibling TXT fallback due to invalid text: " +
                       canonical.error_message);
    return std::nullopt;
  }
  return canonical.text;
}

[[nodiscard]] auto CollectPreviousMonthSiblingTxtFiles(
    const fs::path& source_path, const PreviousMonthInfo& previous_month)
    -> std::vector<PreviousMonthFileCandidate> {
  std::vector<PreviousMonthFileCandidate> candidates;

  std::error_code error;
  const fs::path source_parent = source_path.parent_path();
  if (source_parent.empty() || !fs::exists(source_parent, error) ||
      !fs::is_directory(source_parent, error)) {
    return candidates;
  }

  fs::directory_iterator iterator(source_parent, error);
  if (error) {
    return candidates;
  }

  for (const fs::directory_entry& entry : iterator) {
    if (!entry.is_regular_file()) {
      continue;
    }

    const std::string extension =
        ToLowerAsciiCopy(entry.path().extension().string());
    if (extension != ".txt") {
      continue;
    }

    std::error_code eq_error;
    if (fs::equivalent(entry.path(), source_path, eq_error)) {
      continue;
    }

    const auto content_opt = TryReadTextFile(entry.path());
    if (!content_opt.has_value()) {
      continue;
    }

    const auto month_opt =
        TryParseSingleTxtTargetMonthFromContent(*content_opt);
    if (!month_opt.has_value()) {
      continue;
    }
    if (month_opt->year != previous_month.year ||
        month_opt->month != previous_month.month) {
      continue;
    }

    candidates.push_back(
        PreviousMonthFileCandidate{.path = entry.path(), .content = *content_opt});
  }

  std::ranges::sort(
      candidates, [](const PreviousMonthFileCandidate& left,
                     const PreviousMonthFileCandidate& right) -> bool {
        return left.path.string() < right.path.string();
      });
  return candidates;
}

[[nodiscard]] auto IsBetterTailCandidate(const ActivityTailCandidate& candidate,
                                         const ActivityTailCandidate& best)
    -> bool {
  if (candidate.date != best.date) {
    return candidate.date > best.date;
  }
  if (candidate.end_timestamp != best.end_timestamp) {
    return candidate.end_timestamp > best.end_timestamp;
  }
  return candidate.end_time > best.end_time;
}

[[nodiscard]] auto TryExtractLatestTailFromProcessedData(
    const std::map<std::string, std::vector<DailyLog>>& processed_data,
    std::string_view month_key)
    -> std::optional<app_ports::PreviousActivityTail> {
  std::optional<ActivityTailCandidate> best;

  for (const auto& [key, days] : processed_data) {
    if (key != month_key) {
      continue;
    }

    for (const auto& day : days) {
      for (const auto& activity : day.processedActivities) {
        if (activity.end_time_str.empty()) {
          continue;
        }
        ActivityTailCandidate candidate{
            .date = day.date,
            .end_time = activity.end_time_str,
            .end_timestamp = activity.end_timestamp,
        };
        if (!best.has_value() || IsBetterTailCandidate(candidate, *best)) {
          best = std::move(candidate);
        }
      }
    }
  }

  if (!best.has_value()) {
    return std::nullopt;
  }
  return app_ports::PreviousActivityTail{
      .date = best->date,
      .end_time = best->end_time,
  };
}

[[nodiscard]] auto TryGetPreviousTailFromSiblingTxt(
    const PipelineSession& context, const SingleTxtTargetMonth& target_month)
    -> std::optional<app_ports::PreviousActivityTail> {
  if (context.state.ingest_inputs.size() != 1U) {
    return std::nullopt;
  }

  const auto previous_month =
      TryBuildPreviousMonthInfo(target_month.year, target_month.month);
  if (!previous_month.has_value()) {
    return std::nullopt;
  }

  const auto& input = context.state.ingest_inputs.front();
  if (input.source_id.empty()) {
    return std::nullopt;
  }

  const fs::path source_path(input.source_id);
  if (source_path.empty()) {
    return std::nullopt;
  }

  auto candidates =
      CollectPreviousMonthSiblingTxtFiles(source_path, *previous_month);
  if (candidates.empty()) {
    return std::nullopt;
  }

  if (candidates.size() > 1U) {
    app_ports::LogWarn(
        "[LogLinker] Found multiple sibling TXT files for previous month " +
        previous_month->month_key +
        ". Fallback uses lexicographically first path: " +
        candidates.front().path.string());
  }

  const auto& selected = candidates.front();
  LogProcessor processor(context.state.converter_config);
  auto conversion_result =
      processor.ProcessSourceContent(selected.path.string(), selected.content);
  if (!conversion_result.success || conversion_result.processed_data.empty()) {
    app_ports::LogWarn("[LogLinker] Failed to convert sibling TXT fallback: " +
                       selected.path.string());
    return std::nullopt;
  }

  if (!IsSingleMonthConsistent(conversion_result.processed_data,
                               previous_month->month_key)) {
    app_ports::LogWarn(
        "[LogLinker] Sibling TXT fallback is not single-month consistent: " +
        selected.path.string());
    return std::nullopt;
  }

  const auto tail = TryExtractLatestTailFromProcessedData(
      conversion_result.processed_data, previous_month->month_key);
  if (!tail.has_value()) {
    app_ports::LogWarn("[LogLinker] Sibling TXT fallback has no activity tail: " +
                       selected.path.string());
    return std::nullopt;
  }

  app_ports::LogInfo("[LogLinker] Loaded previous tail from sibling TXT: " +
                     selected.path.string());
  return tail;
}

[[nodiscard]] auto ResolvePreviousTailForReplaceMonth(
    const PipelineSession& context, const SingleTxtTargetMonth& target_month,
    const std::map<std::string, std::vector<DailyLog>>& processed_data,
    app_ports::ITimeSheetRepository& repository)
    -> std::optional<app_ports::PreviousActivityTail> {
  if (processed_data.empty()) {
    return std::nullopt;
  }

  auto previous_tail = repository.TryGetLatestActivityTailBeforeDate(
      target_month.month_start_date);
  if (previous_tail.has_value()) {
    return previous_tail;
  }
  return TryGetPreviousTailFromSiblingTxt(context, target_month);
}

}  // namespace

auto PipelineWorkflow::RunIngest(const std::string& source_path,
                                 DateCheckMode date_check_mode,
                                 bool save_processed,
                                 IngestMode ingest_mode) -> void {
  app_ports::LogInfo("\n--- 启动数据摄入 (Ingest) ---");
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
    app_ports::LogError("\n=== Ingest 执行失败 ===");
    throw std::runtime_error(
        BuildPipelineFailureMessage("Ingestion process failed."));
  }

  auto& context = *result_context_opt;
  app_ports::LogInfo("\n--- 流水线验证通过，准备入库 ---");

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
      app_ports::LogWarn(
          "[LogLinker] No previous-month tail context found (DB/sibling "
          "TXT). Ingest will proceed without cross-month backfill.");
    }

    RunDatabaseImportFromMemoryReplacingMonth(context.result.processed_data,
                                             target_month->year,
                                             target_month->month);
    app_ports::LogInfo("\n=== Ingest 执行成功（单月替换）===");
    return;
  }

  if (!context.result.processed_data.empty()) {
    RunDatabaseImportFromMemory(context.result.processed_data);
    app_ports::LogInfo("\n=== Ingest 执行成功 ===");
  } else {
    app_ports::LogWarn("\n=== Ingest 完成但无数据产生 ===");
  }
}

auto PipelineWorkflow::RunIngestReplacingAll(const std::string& source_path,
                                             DateCheckMode date_check_mode,
                                             bool save_processed) -> void {
  app_ports::LogInfo("\n--- 启动数据摄入 (Replace All) ---");
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
    app_ports::LogError("\n=== Replace-all ingest 执行失败 ===");
    throw std::runtime_error(
        BuildPipelineFailureMessage("Replace-all ingestion process failed."));
  }

  auto& context = *result_context_opt;
  app_ports::LogInfo("\n--- 流水线验证通过，准备全量替换入库 ---");
  RunDatabaseImportFromMemoryReplacingAll(context.result.processed_data);
  app_ports::LogInfo("\n=== Ingest 执行成功（全量替换）===");
}

}  // namespace tracer::core::application::pipeline
