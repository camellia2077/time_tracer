// application/workflow_handler_import_flow.cpp
#include <algorithm>
#include <cctype>
#include <format>
#include <fstream>
#include <optional>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "application/pipeline/pipeline_manager.hpp"
#include "application/ports/logger.hpp"
#include "application/workflow_handler.hpp"
#include "domain/logic/converter/convert/core/converter_core.hpp"
#include "domain/logic/converter/log_processor.hpp"
#include "domain/ports/diagnostics.hpp"
#include "shared/types/ansi_colors.hpp"
#include "shared/utils/string_utils.hpp"

using namespace core::pipeline;
namespace app_ports = tracer_core::application::ports;

namespace workflow_handler_internal {
[[nodiscard]] auto BuildPipelineFailureMessage(std::string_view base_message)
    -> std::string;
}  // namespace workflow_handler_internal

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
    ch = static_cast<char>(
        std::tolower(static_cast<unsigned char>(ch)));  // ASCII-only
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
        const int kValue = std::stoi(trimmed.substr(1));
        if (kValue < 1 || kValue > kMaxMonth) {
          return std::nullopt;
        }
        month = kValue;
      } catch (const std::exception&) {
        return std::nullopt;
      }
      continue;
    }
  }

  if (!year.has_value() || !month.has_value()) {
    return std::nullopt;
  }

  const std::string kMonthKey = BuildMonthKey(*year, *month);
  return SingleTxtTargetMonth{
      .year = *year,
      .month = *month,
      .month_key = kMonthKey,
      .month_start_date = kMonthKey + "-01",
  };
}

[[nodiscard]] auto TryResolveSingleTxtTargetMonth(
    const PipelineContext& context) -> std::optional<SingleTxtTargetMonth> {
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
  return buffer.str();
}

[[nodiscard]] auto CollectPreviousMonthSiblingTxtFiles(
    const fs::path& source_path, const PreviousMonthInfo& previous_month)
    -> std::vector<PreviousMonthFileCandidate> {
  std::vector<PreviousMonthFileCandidate> candidates;

  std::error_code error;
  const fs::path kSourceParent = source_path.parent_path();
  if (kSourceParent.empty() || !fs::exists(kSourceParent, error) ||
      !fs::is_directory(kSourceParent, error)) {
    return candidates;
  }

  fs::directory_iterator iterator(kSourceParent, error);
  if (error) {
    return candidates;
  }

  for (const fs::directory_entry& entry : iterator) {
    if (!entry.is_regular_file()) {
      continue;
    }

    const std::string kExtension =
        ToLowerAsciiCopy(entry.path().extension().string());
    if (kExtension != ".txt") {
      continue;
    }

    std::error_code eq_error;
    if (fs::equivalent(entry.path(), source_path, eq_error)) {
      continue;
    }

    const auto kContentOpt = TryReadTextFile(entry.path());
    if (!kContentOpt.has_value()) {
      continue;
    }

    const auto kMonthOpt =
        TryParseSingleTxtTargetMonthFromContent(*kContentOpt);
    if (!kMonthOpt.has_value()) {
      continue;
    }
    if (kMonthOpt->year != previous_month.year ||
        kMonthOpt->month != previous_month.month) {
      continue;
    }

    candidates.push_back(PreviousMonthFileCandidate{.path = entry.path(),
                                                    .content = *kContentOpt});
  }

  std::ranges::sort(candidates,
                    [](const PreviousMonthFileCandidate& left,
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
    const PipelineContext& context, const SingleTxtTargetMonth& target_month)
    -> std::optional<app_ports::PreviousActivityTail> {
  if (context.state.ingest_inputs.size() != 1U) {
    return std::nullopt;
  }

  const auto kPreviousMonth =
      TryBuildPreviousMonthInfo(target_month.year, target_month.month);
  if (!kPreviousMonth.has_value()) {
    return std::nullopt;
  }

  const auto& input = context.state.ingest_inputs.front();
  if (input.source_id.empty()) {
    return std::nullopt;
  }

  const fs::path kSourcePath(input.source_id);
  if (kSourcePath.empty()) {
    return std::nullopt;
  }

  auto candidates =
      CollectPreviousMonthSiblingTxtFiles(kSourcePath, *kPreviousMonth);
  if (candidates.empty()) {
    return std::nullopt;
  }

  if (candidates.size() > 1U) {
    app_ports::LogWarn(
        "[LogLinker] Found multiple sibling TXT files for previous month " +
        kPreviousMonth->month_key +
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
                               kPreviousMonth->month_key)) {
    app_ports::LogWarn(
        "[LogLinker] Sibling TXT fallback is not single-month consistent: " +
        selected.path.string());
    return std::nullopt;
  }

  const auto kTail = TryExtractLatestTailFromProcessedData(
      conversion_result.processed_data, kPreviousMonth->month_key);
  if (!kTail.has_value()) {
    app_ports::LogWarn(
        "[LogLinker] Sibling TXT fallback has no activity tail: " +
        selected.path.string());
    return std::nullopt;
  }

  app_ports::LogInfo("[LogLinker] Loaded previous tail from sibling TXT: " +
                     selected.path.string());
  return kTail;
}

[[nodiscard]] auto ResolvePreviousTailForReplaceMonth(
    const PipelineContext& context, const SingleTxtTargetMonth& target_month,
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

[[nodiscard]] auto BuildIngestOptions(DateCheckMode date_check_mode,
                                      bool save_processed) -> AppOptions {
  AppOptions options;
  options.validate_structure = true;
  options.convert = true;
  options.validate_logic = true;
  options.date_check_mode = date_check_mode;
  options.save_processed_output = save_processed;
  return options;
}

}  // namespace

auto WorkflowHandler::RunIngest(const std::string& source_path,
                                DateCheckMode date_check_mode,
                                bool save_processed, IngestMode ingest_mode)
    -> void {
  app_ports::LogInfo("\n--- 启动数据摄入 (Ingest) ---");
  tracer_core::domain::ports::ClearBufferedDiagnostics();

  const auto kDbCheck = database_health_checker_->CheckReady();
  if (!kDbCheck.ok) {
    throw std::runtime_error(kDbCheck.message.empty()
                                 ? "Database readiness check failed."
                                 : kDbCheck.message);
  }

  PipelineManager pipeline(output_root_path_, converter_config_provider_,
                           ingest_input_provider_, processed_data_storage_,
                           validation_issue_reporter_);
  const AppOptions kFullOptions =
      BuildIngestOptions(date_check_mode, save_processed);
  auto result_context_opt = pipeline.Run(source_path, kFullOptions);

  namespace colors = tracer_core::common::colors;
  if (!result_context_opt) {
    app_ports::LogError(std::string(colors::kRed) +
                        "\n=== Ingest 执行失败 ===" + colors::kReset.data());
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
          context.result.processed_data, kPreviousTail->date,
          kPreviousTail->end_time);
    } else if (!context.result.processed_data.empty()) {
      app_ports::LogWarn(
          "[LogLinker] No previous-month tail context found (DB/sibling "
          "TXT). Ingest will proceed without cross-month backfill.");
    }

    RunDatabaseImportFromMemoryReplacingMonth(
        context.result.processed_data, kTargetMonth->year, kTargetMonth->month);
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
}
