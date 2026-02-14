// application/parser/memory_parser.cpp
#include "application/parser/memory_parser.hpp"

#include <iostream>

namespace {
constexpr size_t kDateStrMinLen = 7;
constexpr size_t kDateStrYearEnd = 4;
constexpr size_t kDateStrMonthStart = 5;
constexpr size_t kDateStrMonthLen = 2;

auto ProcessDayDate(const std::string& date_str, DayData& day_data) -> void {
  if (date_str.length() >= kDateStrMinLen) {
    try {
      day_data.year = std::stoi(date_str.substr(0, kDateStrYearEnd));
      day_data.month =
          std::stoi(date_str.substr(kDateStrMonthStart, kDateStrMonthLen));
    } catch (...) {
      day_data.year = 0;
      day_data.month = 0;
    }
  } else {
    day_data.year = 0;
    day_data.month = 0;
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

auto ResolveGetupTime(const DailyLog& day) -> std::string {
  if (day.isContinuation) {
    return "Null";
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

  for (const auto& [month_str, days] : data_map) {
    for (const auto& input_day : days) {
      DayData day_data;
      day_data.date = input_day.date;

      ProcessDayDate(input_day.date, day_data);

      day_data.status = input_day.hasStudyActivity ? 1 : 0;
      day_data.sleep = input_day.hasSleepActivity ? 1 : 0;
      day_data.exercise = input_day.hasExerciseActivity ? 1 : 0;
      day_data.remark = ProcessDayRemarks(input_day.generalRemarks);
      day_data.getup_time = ResolveGetupTime(input_day);

      // [核心修改] 直接赋值统计数据！
      day_data.stats = input_day.stats;

      all_data.days.push_back(day_data);

      for (const auto& activity : input_day.processedActivities) {
        TimeRecordInternal record;

        // [核心修改] 利用 BaseActivityRecord 的赋值
        // record 是 TimeRecordInternal，继承自 BaseActivityRecord
        // activity 也是 BaseActivityRecord
        static_cast<BaseActivityRecord&>(record) = activity;

        // 补充特有字段
        record.date = input_day.date;

        all_data.records.push_back(record);
      }
    }
  }
  return all_data;
}
