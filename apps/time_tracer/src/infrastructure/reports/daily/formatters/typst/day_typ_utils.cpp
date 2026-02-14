// infrastructure/reports/daily/formatters/typst/day_typ_utils.cpp
#include "infrastructure/reports/daily/formatters/typst/day_typ_utils.hpp"

#include <string>

#include "infrastructure/reports/shared/formatters/typst/typ_utils.hpp"
#include "infrastructure/reports/shared/utils/format/report_string_utils.hpp"
#include "infrastructure/reports/shared/utils/format/time_format.hpp"

namespace {
constexpr std::size_t kBulletLineReservePadding = 8;
constexpr size_t kActivityBaseLinePadding = 64;
constexpr size_t kColoredActivityLinePadding = 20;

auto BuildBulletLine(const std::string& label, const std::string& value)
    -> std::string {
  std::string line;
  line.reserve(label.size() + value.size() + kBulletLineReservePadding);
  line += "+ *";
  line += label;
  line += ":* ";
  line += value;
  return line;
}

auto BuildActivityLine(const TimeRecord& record,
                       const std::shared_ptr<DayTypConfig>& config)
    -> std::string {
  std::string project_path =
      ReplaceAll(record.project_path, "_", config->GetActivityConnector());

  std::string base_line;
  base_line.reserve(project_path.size() + kActivityBaseLinePadding);
  base_line += record.start_time;
  base_line += " - ";
  base_line += record.end_time;
  base_line += " (";
  base_line += TimeFormatDuration(record.duration_seconds);
  base_line += "): ";
  base_line += project_path;

  std::string output = "+ " + base_line;
  for (const auto& pair : config->GetKeywordColors()) {
    if (record.project_path.find(pair.first) == std::string::npos) {
      continue;
    }
    output.clear();
    output.reserve(base_line.size() + pair.second.size() +
                   kColoredActivityLinePadding);
    output += "+ #text(rgb(\"";
    output += pair.second;
    output += "\"))[";
    output += base_line;
    output += "]";
    break;
  }

  if (record.activityRemark.has_value()) {
    std::string formatted_activity_remark =
        FormatMultilineForList(record.activityRemark.value(), 4, " \\");
    output += "\n  ";
    output += BuildBulletLine(config->GetActivityRemarkLabel(),
                              formatted_activity_remark);
  }

  return output;
}

}  // namespace

namespace DayTypUtils {

void DisplayHeader(std::string& report_stream, const DailyReportData& data,
                   const std::shared_ptr<DayTypConfig>& config) {
  std::string title_text = config->GetTitlePrefix();
  title_text += " ";
  title_text += data.date;
  report_stream += TypUtils::BuildTitleText(
      config->GetTitleFont(), config->GetReportTitleFontSize(), title_text);
  report_stream += "\n\n";

  report_stream += BuildBulletLine(config->GetDateLabel(), data.date);
  report_stream += "\n";
  report_stream += BuildBulletLine(config->GetTotalTimeLabel(),
                                   TimeFormatDuration(data.total_duration));
  report_stream += "\n";
  report_stream += BuildBulletLine(config->GetStatusLabel(),
                                   BoolToString(data.metadata.status));
  report_stream += "\n";
  report_stream += BuildBulletLine(config->GetSleepLabel(),
                                   BoolToString(data.metadata.sleep));
  report_stream += "\n";
  report_stream += BuildBulletLine(config->GetExerciseLabel(),
                                   BoolToString(data.metadata.exercise));
  report_stream += "\n";
  report_stream +=
      BuildBulletLine(config->GetGetupTimeLabel(), data.metadata.getup_time);
  report_stream += "\n";

  std::string formatted_remark =
      FormatMultilineForList(data.metadata.remark, 2, " \\");
  report_stream += BuildBulletLine(config->GetRemarkLabel(), formatted_remark);
  report_stream += "\n";
}

void DisplayDetailedActivities(std::string& report_stream,
                               const DailyReportData& data,
                               const std::shared_ptr<DayTypConfig>& config) {
  if (data.detailed_records.empty()) {
    return;
  }

  report_stream += TypUtils::BuildTitleText(config->GetCategoryTitleFont(),
                                            config->GetCategoryTitleFontSize(),
                                            config->GetAllActivitiesLabel());
  report_stream += "\n\n";

  for (const auto& record : data.detailed_records) {
    report_stream += BuildActivityLine(record, config);
    report_stream += "\n";
  }
}

}  // namespace DayTypUtils
