// domain/logic/converter/convert/facade/converter_service.cpp
#include "application/service/converter_service.hpp"

#include "application/parser/text_parser.hpp"
#include "domain/logic/converter/convert/core/day_processor.hpp"
#include "shared/types/ansi_colors.hpp"

namespace {
constexpr size_t kIsoDateLength = 10;
constexpr size_t kIsoYearLength = 4;
constexpr size_t kIsoMonthOffset = 5;
constexpr size_t kIsoMonthLength = 2;
}  // namespace

ConverterService::ConverterService(const ConverterConfig& config)
    : config_(config) {}

auto ConverterService::IsDuplicateAcrossYear(const DailyLog& previous_day,
                                             const DailyLog& current_day)
    -> bool {
  if (previous_day.date.length() != kIsoDateLength ||
      current_day.date.length() != kIsoDateLength) {
    return false;
  }

  // Example: "2021-12-31" vs "2022-01-01"
  std::string prev_month =
      previous_day.date.substr(kIsoMonthOffset, kIsoMonthLength);
  std::string curr_month =
      current_day.date.substr(kIsoMonthOffset, kIsoMonthLength);

  if (prev_month == "12" && curr_month == "01") {
    int prev_year = std::stoi(previous_day.date.substr(0, kIsoYearLength));
    int curr_year = std::stoi(current_day.date.substr(0, kIsoYearLength));
    return curr_year == prev_year + 1;
  }

  return false;
}

auto ConverterService::ExecuteConversion(
    std::istream& combined_input_stream,
    std::function<void(DailyLog&&)> data_consumer,
    std::string_view source_file) -> void {
  TextParser parser(config_);
  DayProcessor processor(config_);

  // 滑动窗口：只在内存中保留前一天
  DailyLog previous_day;
  bool has_previous = false;

  parser.Parse(combined_input_stream, [&](DailyLog& current_day) -> void {
    // 1. 初始化 / 跨天逻辑处理
    if (!has_previous) {
      DailyLog empty_day;
      processor.Process(empty_day, current_day);

    } else {
      // 利用 current_day 完善 previous_day 的睡眠逻辑
      processor.Process(previous_day, current_day);

      // 2. 去重逻辑与回调移交
      if (!IsDuplicateAcrossYear(previous_day, current_day)) {
        if (data_consumer != nullptr) {
          data_consumer(std::move(previous_day));
        }
      }
    }

    // 3. 滑动窗口：current_day 变为 previous_day
    previous_day = std::move(current_day);
    has_previous = true;
  }, source_file);

  // 4. 处理最后一天 (Flush)
  if (has_previous) {
    DailyLog empty_next_day;
    processor.Process(previous_day, empty_next_day);

    if (data_consumer != nullptr) {
      data_consumer(std::move(previous_day));
    }
  }
}
