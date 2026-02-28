// application/parser/memory_parser.cpp
#include "application/parser/memory_parser.hpp"

#include <optional>

#include "domain/ports/diagnostics.hpp"

namespace {
constexpr size_t kDateStrMinLen = 7;
constexpr size_t kDateStrYearEnd = 4;
constexpr size_t kDateStrMonthStart = 5;
constexpr size_t kDateStrMonthLen = 2;

auto TryParseYearMonth(const std::string& date_str)
    -> std::optional<std::pair<int, int>> {
  if (date_str.length() < kDateStrMinLen) {
    return std::nullopt;
  }

  try {
    const int kYear = std::stoi(date_str.substr(0, kDateStrYearEnd));
    const int kMonth =
        std::stoi(date_str.substr(kDateStrMonthStart, kDateStrMonthLen));
    return std::pair{kYear, kMonth};
  } catch (const std::exception&) {
    return std::nullopt;
  }
}

auto ProcessDayRemarks(const std::vector<std::string>& remarks_vec)
    -> std::string {
  std::string merged_remark;
  for (size_t i = 0; i < remarks_vec.size(); ++i) {
    merged_remark += remarks_vec[i];
    if (i < remarks_vec.size() - 1) {
      merged_remark += "\n";
    }
  }
  return merged_remark;
}

auto ResolveGetupTime(const DailyLog& day) -> std::optional<std::string> {
  if (day.isContinuation) {
    return std::nullopt;
  }
  if (day.getupTime.empty()) {
    return "00:00";
  }
  return day.getupTime;
}
}  // namespace

auto MemoryParser::Parse(
    const std::map<std::string, std::vector<DailyLog>>& data_map)
    -> ParsedData {
  ParsedData all_data;
  size_t expected_days = 0;
  size_t expected_records = 0;

  for (const auto& [bucket_key, days] : data_map) {
    static_cast<void>(bucket_key);
    expected_days += days.size();
    for (const auto& day : days) {
      expected_records += day.processedActivities.size();
    }
  }

  all_data.days.reserve(expected_days);
  all_data.records.reserve(expected_records);

  for (const auto& [year_month_bucket, days] : data_map) {
    // Key (`YYYY-MM`) is a grouping bucket from upstream pipeline.
    // Conversion only depends on day payload values.
    static_cast<void>(year_month_bucket);
    for (const auto& input_day : days) {
      const auto kParsedYearMonth = TryParseYearMonth(input_day.date);
      if (!kParsedYearMonth.has_value()) {
        tracer_core::domain::ports::EmitWarn(
            "[memory parser] skip day with invalid date: " + input_day.date);
        continue;
      }

      DayData day_data;
      day_data.date = input_day.date;
      day_data.year = kParsedYearMonth->first;
      day_data.month = kParsedYearMonth->second;

      day_data.status = input_day.hasStudyActivity ? 1 : 0;
      day_data.sleep = input_day.hasSleepActivity ? 1 : 0;
      day_data.exercise = input_day.hasExerciseActivity ? 1 : 0;
      day_data.remark = ProcessDayRemarks(input_day.generalRemarks);
      day_data.getup_time = ResolveGetupTime(input_day);

      // [核心修改] 直接赋值统计数据！
      day_data.stats = input_day.stats;

      all_data.days.push_back(std::move(day_data));

      for (const auto& activity : input_day.processedActivities) {
        TimeRecordInternal record;
        record.logical_id = activity.logical_id;
        record.start_timestamp = activity.start_timestamp;
        record.end_timestamp = activity.end_timestamp;
        record.start_time_str = activity.start_time_str;
        record.end_time_str = activity.end_time_str;
        record.project_path = activity.project_path;
        record.duration_seconds = activity.duration_seconds;
        record.remark = activity.remark;
        record.date = input_day.date;

        all_data.records.push_back(std::move(record));
      }
    }
  }
  return all_data;
}
