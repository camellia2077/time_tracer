// importer/parser/details/day_parser.cpp
#include "day_parser.hpp"

#include <stdexcept>

namespace {
constexpr size_t kFullDateStrLen = 10;
constexpr size_t kDateStrYearEnd = 4;
constexpr size_t kDateStrMonthStart = 5;
constexpr size_t kDateStrMonthLen = 2;
}  // namespace

auto DayParser::parse(const nlohmann::json& day_json) -> DayData {
  try {
    const auto& headers = day_json.at("headers");
    const auto& generated_stats = day_json.at("generated_stats");

    DayData day_data;
    day_data.date = headers.at("date");
    day_data.status = headers.at("status");
    day_data.sleep = headers.at("sleep");
    day_data.getup_time = headers.value("getup", "00:00");
    day_data.remark = headers.at("remark");
    day_data.exercise = headers.value("exercise", 0);

    // [核心修改] 填充 stats 结构体
    // 注意 JSON key 已经在 Converter 重构中改为 snake_case，这里直接匹配
    day_data.stats.total_exercise_time =
        generated_stats.value("total_exercise_time", 0);
    day_data.stats.anaerobic_time = generated_stats.value("anaerobic_time", 0);
    day_data.stats.cardio_time = generated_stats.value("cardio_time", 0);

    day_data.stats.gaming_time = generated_stats.value("gaming_time", 0);
    day_data.stats.grooming_time = generated_stats.value("grooming_time", 0);
    day_data.stats.toilet_time = generated_stats.value("toilet_time", 0);

    day_data.stats.sleep_night_time =
        generated_stats.value("sleep_night_time", 0);
    day_data.stats.sleep_day_time = generated_stats.value("sleep_day_time", 0);
    day_data.stats.sleep_total_time =
        generated_stats.value("sleep_total_time", 0);

    day_data.stats.recreation_time =
        generated_stats.value("recreation_time", 0);
    day_data.stats.recreation_zhihu_time =
        generated_stats.value("recreation_zhihu_time", 0);
    day_data.stats.recreation_bilibili_time =
        generated_stats.value("recreation_bilibili_time", 0);
    day_data.stats.recreation_douyin_time =
        generated_stats.value("recreation_douyin_time", 0);

    day_data.stats.study_time = generated_stats.value("total_study_time", 0);

    if (day_data.date.length() == kFullDateStrLen) {
      day_data.year = std::stoi(day_data.date.substr(0, kDateStrYearEnd));
      day_data.month =
          std::stoi(day_data.date.substr(kDateStrMonthStart, kDateStrMonthLen));
    } else {
      day_data.year = 0;
      day_data.month = 0;
    }
    return day_data;

  } catch (const nlohmann::json::out_of_range& e) {
    std::string date_str = day_json.value("headers", nlohmann::json::object())
                               .value("date", "[Unknown Date]");
    throw std::runtime_error("Required JSON key not found for day " + date_str +
                             ": " + e.what());
  }
}