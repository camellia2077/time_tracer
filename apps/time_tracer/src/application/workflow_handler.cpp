// application/workflow_handler.cpp
#include "application/workflow_handler.hpp"

#include <algorithm>
#include <cctype>
#include <format>
#include <fstream>
#include <iomanip>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "application/importer/import_service.hpp"
#include "application/pipeline/pipeline_manager.hpp"
#include "application/ports/logger.hpp"
#include "domain/logic/converter/convert/core/converter_core.hpp"
#include "domain/logic/converter/log_processor.hpp"
#include "domain/ports/diagnostics.hpp"
#include "shared/types/ansi_colors.hpp"
#include "shared/utils/string_utils.hpp"

namespace fs = std::filesystem;

// 使用之前定义的命名空间
using namespace core::pipeline;
namespace app_ports = time_tracer::application::ports;

namespace {

auto PrintImportStats(const ImportStats& stats, std::string_view title)
    -> void {
  std::ostringstream stream;
  stream << "\n--- " << title << " Report ---";
  app_ports::LogInfo(stream.str());

  namespace colors = time_tracer::common::colors;
  if (!stats.db_open_success) {
    app_ports::LogError(
        std::string(colors::kRed) + "[Fatal] DB Error: " +
        (stats.error_message.empty() ? "Unknown" : stats.error_message) +
        colors::kReset.data());
    return;
  }

  if (!stats.transaction_success) {
    app_ports::LogError(std::string(colors::kRed) +
                        "[Fatal] Transaction Failed: " + stats.error_message +
                        colors::kReset.data());
    return;
  }

  const bool kHasSkipped =
      (stats.skipped_days > 0U) || (stats.skipped_records > 0U);
  const bool kHasLegacyFailed = !stats.failed_files.empty();

  if (!kHasLegacyFailed && !kHasSkipped) {
    app_ports::LogInfo(std::string(colors::kGreen) + "[Success] Processed " +
                       std::to_string(stats.successful_files) + " items." +
                       colors::kReset.data());
  } else {
    std::ostringstream summary;
    summary << std::string(colors::kYellow)
            << "[Partial] Days=" << stats.successful_days << "/"
            << stats.total_days << ", Records=" << stats.successful_records
            << "/" << stats.total_records;
    if (kHasLegacyFailed) {
      summary << ", LegacyFailedFiles=" << stats.failed_files.size();
    }
    summary << colors::kReset.data();
    app_ports::LogWarn(summary.str());

    for (const auto& failed_file : stats.failed_files) {
      app_ports::LogError("  Failed: " + failed_file);
    }
  }

  {
    std::ostringstream day_record_breakdown;
    day_record_breakdown << "Breakdown: days(total=" << stats.total_days
                         << ", success=" << stats.successful_days
                         << ", skipped=" << stats.skipped_days
                         << "), records(total=" << stats.total_records
                         << ", success=" << stats.successful_records
                         << ", skipped=" << stats.skipped_records << ")";
    app_ports::LogInfo(day_record_breakdown.str());
  }

  if (!stats.reason_buckets.empty()) {
    app_ports::LogInfo("Reason Buckets:");
    for (const auto& [reason, count] : stats.reason_buckets) {
      app_ports::LogInfo("  - " + reason + ": " + std::to_string(count));
    }
  }

  const double kTotalTime =
      stats.parsing_duration_s + stats.db_insertion_duration_s;

  std::ostringstream timing;
  timing << std::fixed << std::setprecision(3)
         << "Timing: Parse=" << stats.parsing_duration_s
         << "s, Insert=" << stats.db_insertion_duration_s
         << "s, Total=" << kTotalTime << "s";
  app_ports::LogInfo(timing.str());

  if (stats.replaced_month.has_value()) {
    app_ports::LogInfo("Replace scope: month=" + *stats.replaced_month);
  }
}

[[nodiscard]] auto BuildPipelineFailureMessage(std::string_view base_message)
    -> std::string {
  std::string message(base_message);
  const std::string kReportDestination =
      time_tracer::domain::ports::GetErrorReportDestinationLabel();
  if (!kReportDestination.empty() && kReportDestination != "disabled") {
    message += "\nFull error report: " + kReportDestination;
  }
  return message;
}

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

[[nodiscard]] auto ToLowerAscii(std::string value) -> std::string {
  for (char& ch : value) {
    ch = static_cast<char>(
        std::tolower(static_cast<unsigned char>(ch)));  // ASCII-only
  }
  return value;
}

[[nodiscard]] auto BuildMonthKey(const int year, const int month)
    -> std::string {
  return std::format("{:04d}-{:02d}", year, month);
}

[[nodiscard]] auto BuildPreviousMonthInfo(const int year, const int month)
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

[[nodiscard]] auto ParseSingleTxtTargetMonthFromContent(
    std::string_view content) -> std::optional<SingleTxtTargetMonth> {
  std::stringstream stream{std::string(content)};
  std::string line;
  std::optional<int> year;
  std::optional<int> month;

  while (std::getline(stream, line)) {
    const std::string trimmed = Trim(line);
    if (trimmed.empty()) {
      continue;
    }

    if (!year.has_value() && trimmed.size() == 5 && trimmed[0] == 'y') {
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
        if (value < 1 || value > 12) {
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

  const std::string key = BuildMonthKey(*year, *month);
  return SingleTxtTargetMonth{
      .year = *year,
      .month = *month,
      .month_key = key,
      .month_start_date = key + "-01",
  };
}

[[nodiscard]] auto ResolveSingleTxtTargetMonth(const PipelineContext& context)
    -> std::optional<SingleTxtTargetMonth> {
  if (context.state.ingest_inputs.size() != 1U) {
    return std::nullopt;
  }
  return ParseSingleTxtTargetMonthFromContent(
      context.state.ingest_inputs.front().content);
}

[[nodiscard]] auto EnsureSingleMonthConsistency(
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

[[nodiscard]] auto ReadTextFile(const fs::path& path)
    -> std::optional<std::string> {
  std::ifstream input(path, std::ios::in | std::ios::binary);
  if (!input.is_open()) {
    return std::nullopt;
  }

  std::ostringstream buffer;
  buffer << input.rdbuf();
  return buffer.str();
}

[[nodiscard]] auto CollectPreviousMonthSiblingFiles(
    const fs::path& source_path, const PreviousMonthInfo& previous_month)
    -> std::vector<PreviousMonthFileCandidate> {
  std::vector<PreviousMonthFileCandidate> candidates;

  std::error_code error;
  const fs::path source_parent = source_path.parent_path();
  if (source_parent.empty() || !fs::exists(source_parent, error) ||
      !fs::is_directory(source_parent, error)) {
    return candidates;
  }

  fs::directory_iterator it(source_parent, error);
  if (error) {
    return candidates;
  }

  for (const fs::directory_entry& entry : it) {
    if (!entry.is_regular_file()) {
      continue;
    }

    const std::string extension =
        ToLowerAscii(entry.path().extension().string());
    if (extension != ".txt") {
      continue;
    }

    std::error_code eq_error;
    if (fs::equivalent(entry.path(), source_path, eq_error)) {
      continue;
    }

    const auto content_opt = ReadTextFile(entry.path());
    if (!content_opt.has_value()) {
      continue;
    }

    const auto month_opt = ParseSingleTxtTargetMonthFromContent(*content_opt);
    if (!month_opt.has_value()) {
      continue;
    }
    if (month_opt->year != previous_month.year ||
        month_opt->month != previous_month.month) {
      continue;
    }

    candidates.push_back(PreviousMonthFileCandidate{.path = entry.path(),
                                                    .content = *content_opt});
  }

  std::sort(candidates.begin(), candidates.end(),
            [](const PreviousMonthFileCandidate& left,
               const PreviousMonthFileCandidate& right) {
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

[[nodiscard]] auto ExtractLatestTailFromProcessedData(
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
    const PipelineContext& context, const SingleTxtTargetMonth& target_month)
    -> std::optional<app_ports::PreviousActivityTail> {
  if (context.state.ingest_inputs.size() != 1U) {
    return std::nullopt;
  }

  const auto previous_month =
      BuildPreviousMonthInfo(target_month.year, target_month.month);
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
      CollectPreviousMonthSiblingFiles(source_path, *previous_month);
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

  if (!EnsureSingleMonthConsistency(conversion_result.processed_data,
                                    previous_month->month_key)) {
    app_ports::LogWarn(
        "[LogLinker] Sibling TXT fallback is not single-month consistent: " +
        selected.path.string());
    return std::nullopt;
  }

  const auto tail = ExtractLatestTailFromProcessedData(
      conversion_result.processed_data, previous_month->month_key);
  if (!tail.has_value()) {
    app_ports::LogWarn(
        "[LogLinker] Sibling TXT fallback has no activity tail: " +
        selected.path.string());
    return std::nullopt;
  }

  app_ports::LogInfo("[LogLinker] Loaded previous tail from sibling TXT: " +
                     selected.path.string());
  return tail;
}

}  // namespace

WorkflowHandler::WorkflowHandler(
    fs::path output_root_path,
    std::shared_ptr<app_ports::IProcessedDataLoader> processed_data_loader,
    std::shared_ptr<app_ports::ITimeSheetRepository> time_sheet_repository,
    std::shared_ptr<app_ports::IDatabaseHealthChecker> database_health_checker,
    std::shared_ptr<app_ports::IConverterConfigProvider>
        converter_config_provider,
    std::shared_ptr<app_ports::IIngestInputProvider> ingest_input_provider,
    std::shared_ptr<app_ports::IProcessedDataStorage> processed_data_storage,
    std::shared_ptr<app_ports::IValidationIssueReporter>
        validation_issue_reporter)
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
        "WorkflowHandler dependencies must not be null.");
  }
}

WorkflowHandler::~WorkflowHandler() = default;

auto WorkflowHandler::RunConverter(const std::string& input_path,
                                   const AppOptions& options) -> void {
  PipelineManager pipeline(output_root_path_, converter_config_provider_,
                           ingest_input_provider_, processed_data_storage_,
                           validation_issue_reporter_);
  if (!pipeline.Run(input_path, options)) {
    throw std::runtime_error("Converter Pipeline Failed.");
  }
}

void WorkflowHandler::RunDatabaseImport(const std::string& processed_path_str) {
  app_ports::LogInfo("正在解析 JSON 数据...");

  auto load_result = processed_data_loader_->LoadDailyLogs(processed_path_str);

  namespace colors = time_tracer::common::colors;
  for (const auto& error : load_result.errors) {
    app_ports::LogError(std::string(colors::kRed) + "解析文件失败 " +
                        error.source + ": " + error.message +
                        colors::kReset.data());
  }

  if (load_result.data_by_source.empty()) {
    app_ports::LogWarn(std::string(colors::kYellow) +
                       "没有有效的 JSON 数据可供导入。" +
                       colors::kReset.data());
    return;
  }

  RunDatabaseImportFromMemory(load_result.data_by_source);
}

auto WorkflowHandler::RunDatabaseImportFromMemory(
    const std::map<std::string, std::vector<DailyLog>>& data_map) -> void {
  app_ports::LogInfo("Task: Memory Import...");

  ImportService service(*time_sheet_repository_);
  ImportStats stats = service.ImportFromMemory(data_map);

  PrintImportStats(stats, "Memory Import");

  if (!stats.db_open_success || !stats.transaction_success) {
    throw std::runtime_error(stats.error_message.empty()
                                 ? "Memory import failed."
                                 : stats.error_message);
  }
}

auto WorkflowHandler::RunDatabaseImportFromMemoryReplacingMonth(
    const std::map<std::string, std::vector<DailyLog>>& data_map, int year,
    int month) -> void {
  app_ports::LogInfo("Task: Memory Import (Replace Month)...");

  ImportService service(*time_sheet_repository_);
  ImportStats stats = service.ImportFromMemory(
      data_map, ReplaceMonthTarget{.year = year, .month = month});

  PrintImportStats(stats, "Memory Import (Replace Month)");

  if (!stats.db_open_success || !stats.transaction_success) {
    throw std::runtime_error(stats.error_message.empty()
                                 ? "Memory import (replace month) failed."
                                 : stats.error_message);
  }
}

auto WorkflowHandler::RunIngest(const std::string& source_path,
                                DateCheckMode date_check_mode,
                                bool save_processed, IngestMode ingest_mode)
    -> void {
  app_ports::LogInfo("\n--- 启动数据摄入 (Ingest) ---");
  time_tracer::domain::ports::ClearBufferedDiagnostics();

  const auto kDbCheck = database_health_checker_->CheckReady();
  if (!kDbCheck.ok) {
    throw std::runtime_error(kDbCheck.message.empty()
                                 ? "Database readiness check failed."
                                 : kDbCheck.message);
  }

  PipelineManager pipeline(output_root_path_, converter_config_provider_,
                           ingest_input_provider_, processed_data_storage_,
                           validation_issue_reporter_);

  AppOptions full_options;
  full_options.validate_structure = true;
  full_options.convert = true;
  full_options.validate_logic = true;
  full_options.date_check_mode = date_check_mode;
  full_options.save_processed_output = save_processed;

  auto result_context_opt = pipeline.Run(source_path, full_options);

  namespace colors = time_tracer::common::colors;
  if (result_context_opt) {
    auto& context = *result_context_opt;
    app_ports::LogInfo("\n--- 流水线验证通过，准备入库 ---");

    if (ingest_mode == IngestMode::kSingleTxtReplaceMonth) {
      const auto target_month = ResolveSingleTxtTargetMonth(context);
      if (!target_month.has_value()) {
        throw std::runtime_error(
            "Single TXT replace-month ingest requires exactly one TXT input "
            "with valid headers: yYYYY + mMM.");
      }

      if (!EnsureSingleMonthConsistency(context.result.processed_data,
                                        target_month->month_key)) {
        throw std::runtime_error(
            "Single TXT replace-month ingest failed: parsed days are not "
            "consistent with header month " +
            target_month->month_key + ".");
      }

      std::optional<app_ports::PreviousActivityTail> previous_tail;
      if (!context.result.processed_data.empty()) {
        previous_tail =
            time_sheet_repository_->TryGetLatestActivityTailBeforeDate(
                target_month->month_start_date);
        if (!previous_tail.has_value()) {
          previous_tail =
              TryGetPreviousTailFromSiblingTxt(context, *target_month);
        }

        if (previous_tail.has_value()) {
          LogLinker linker(context.state.converter_config);
          linker.LinkFirstDayWithExternalPreviousEvent(
              context.result.processed_data, previous_tail->date,
              previous_tail->end_time);
        } else {
          app_ports::LogWarn(
              "[LogLinker] No previous-month tail context found (DB/sibling "
              "TXT). Ingest will proceed without cross-month backfill.");
        }
      }

      RunDatabaseImportFromMemoryReplacingMonth(context.result.processed_data,
                                                target_month->year,
                                                target_month->month);
      app_ports::LogInfo(
          std::string(colors::kGreen) +
          "\n=== Ingest 执行成功（单月替换）===" + colors::kReset.data());
      return;
    }

    if (!context.result.processed_data.empty()) {
      RunDatabaseImportFromMemory(context.result.processed_data);
      app_ports::LogInfo(std::string(colors::kGreen) +
                         "\n=== Ingest 执行成功 ===" + colors::kReset.data());
    } else {
      app_ports::LogWarn(
          std::string(colors::kYellow) +
          "\n=== Ingest 完成但无数据产生 ===" + colors::kReset.data());
    }
    return;
  }

  app_ports::LogError(std::string(colors::kRed) +
                      "\n=== Ingest 执行失败 ===" + colors::kReset.data());
  throw std::runtime_error(
      BuildPipelineFailureMessage("Ingestion process failed."));
}

auto WorkflowHandler::RunValidateStructure(const std::string& source_path)
    -> void {
  time_tracer::domain::ports::ClearBufferedDiagnostics();
  AppOptions options;
  options.input_path = source_path;
  options.validate_structure = true;
  options.convert = false;
  options.validate_logic = false;
  options.save_processed_output = false;
  options.date_check_mode = DateCheckMode::kNone;

  PipelineManager pipeline(output_root_path_, converter_config_provider_,
                           ingest_input_provider_, processed_data_storage_,
                           validation_issue_reporter_);
  if (!pipeline.Run(source_path, options)) {
    throw std::runtime_error(
        BuildPipelineFailureMessage("Validate structure pipeline failed."));
  }
}

auto WorkflowHandler::RunValidateLogic(const std::string& source_path,
                                       DateCheckMode date_check_mode) -> void {
  time_tracer::domain::ports::ClearBufferedDiagnostics();
  AppOptions options;
  options.input_path = source_path;
  options.validate_structure = false;
  options.convert = true;
  options.validate_logic = true;
  options.save_processed_output = false;
  options.date_check_mode = date_check_mode;

  PipelineManager pipeline(output_root_path_, converter_config_provider_,
                           ingest_input_provider_, processed_data_storage_,
                           validation_issue_reporter_);
  if (!pipeline.Run(source_path, options)) {
    throw std::runtime_error(
        BuildPipelineFailureMessage("Validate logic pipeline failed."));
  }
}
