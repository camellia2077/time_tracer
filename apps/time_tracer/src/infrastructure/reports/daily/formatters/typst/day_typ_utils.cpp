// infrastructure/reports/daily/formatters/typst/day_typ_utils.cpp
#include "infrastructure/reports/daily/formatters/typst/day_typ_utils.hpp"

#include <algorithm>
#include <format>
#include <iomanip>
#include <string>
#include <vector>

#include "infrastructure/reports/shared/utils/format/bool_to_string.hpp"
#include "infrastructure/reports/shared/utils/format/report_string_utils.hpp"
#include "infrastructure/reports/shared/utils/format/time_format.hpp"

namespace {
auto FormatActivityLine(const TimeRecord& record,
                        const std::shared_ptr<DayTypConfig>& config)
    -> std::string {
  std::string project_path =
      ReplaceAll(record.project_path, "_", config->GetActivityConnector());
  std::string base_string =
      std::format("{} - {} ({}): {}", record.start_time, record.end_time,
                  TimeFormatDuration(record.duration_seconds), project_path);

  // 1. 尝试匹配颜色关键字
  for (const auto& pair : config->GetKeywordColors()) {
    if (record.project_path.find(pair.first) != std::string::npos) {
      const std::string& hex_color = pair.second;
      std::string typst_color_format = std::format(R"(rgb("{}"))", hex_color);
      std::string final_output =
          std::format("+ #text({})[{}]", typst_color_format, base_string);

      if (record.activityRemark.has_value()) {
        // [核心修改] 传入 " \\" 作为后缀，强制 Typst 换行
        std::string formatted_activity_remark =
            FormatMultilineForList(record.activityRemark.value(),
                                   4,     // 缩进 4 个空格
                                   " \\"  // 行尾追加 " \"
            );
        final_output +=
            std::format("\n  + *{}:* {}", config->GetActivityRemarkLabel(),
                        formatted_activity_remark);
      }
      return final_output;
    }
  }

  // 2. 默认格式（无颜色）
  std::string final_output = "+ " + base_string;
  if (record.activityRemark.has_value()) {
    // [核心修改] 传入 " \\" 作为后缀，强制 Typst 换行
    std::string formatted_activity_remark =
        FormatMultilineForList(record.activityRemark.value(),
                               4,     // 缩进 4 个空格
                               " \\"  // 行尾追加 " \"
        );
    final_output +=
        std::format("\n  + *{}:* {}", config->GetActivityRemarkLabel(),
                    formatted_activity_remark);
  }

  return final_output;
}

}  // namespace

namespace DayTypUtils {

void DisplayHeader(std::stringstream& report_stream,
                   const DailyReportData& data,
                   const std::shared_ptr<DayTypConfig>& config) {
  std::string title = std::format(
      R"(#text(font: "{}", size: {}pt)[= {} {}])", config->GetTitleFont(),
      config->GetReportTitleFontSize(), config->GetTitlePrefix(), data.date);
  report_stream << title << "\n\n";
  report_stream << std::format("+ *{}:* {}\n", config->GetDateLabel(),
                               data.date);
  report_stream << std::format("+ *{}:* {}\n", config->GetTotalTimeLabel(),
                               TimeFormatDuration(data.total_duration));
  report_stream << std::format("+ *{}:* {}\n", config->GetStatusLabel(),
                               BoolToString(data.metadata.status));
  report_stream << std::format("+ *{}:* {}\n", config->GetSleepLabel(),
                               BoolToString(data.metadata.sleep));
  report_stream << std::format("+ *{}:* {}\n", config->GetExerciseLabel(),
                               BoolToString(data.metadata.exercise));
  report_stream << std::format("+ *{}:* {}\n", config->GetGetupTimeLabel(),
                               data.metadata.getup_time);

  // [核心修改] 传入 " \\" 作为后缀，强制 Typst 换行
  // 缩进 2 个空格以适配一级列表
  std::string formatted_remark =
      FormatMultilineForList(data.metadata.remark, 2, " \\");
  report_stream << std::format("+ *{}:* {}\n", config->GetRemarkLabel(),
                               formatted_remark);
}

void DisplayDetailedActivities(std::stringstream& report_stream,
                               const DailyReportData& data,
                               const std::shared_ptr<DayTypConfig>& config) {
  if (!data.detailed_records.empty()) {
    report_stream << std::format(R"(#text(font: "{}", size: {}pt)[= {}])",
                                 config->GetCategoryTitleFont(),
                                 config->GetCategoryTitleFontSize(),
                                 config->GetAllActivitiesLabel())
                  << "\n\n";
    for (const auto& record : data.detailed_records) {
      report_stream << FormatActivityLine(record, config) << "\n";
    }
  }
}

}  // namespace DayTypUtils