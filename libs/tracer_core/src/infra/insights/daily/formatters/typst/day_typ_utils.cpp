// infra/insights/daily/formatters/typst/day_typ_utils.cpp
#include "infra/insights/daily/formatters/typst/day_typ_utils.hpp"

#include <string>

#include "infra/insights/shared/formatters/typst/typ_utils.hpp"
#include "infra/insights/shared/utils/format/insights_string_utils.hpp"
#include "infra/insights/shared/utils/format/status_statistics_format.hpp"
#include "infra/insights/shared/utils/format/time_format.hpp"

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
  base_line += FormatClockTime(record.start_time);
  base_line += " - ";
  base_line += FormatClockTime(record.end_time);
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
    // Keep the label separate from the remark body. This makes multiline
    // remarks a continuation of the activity detail instead of a long label
    // line, while the trailing Typst slash preserves each user-entered break.
    output += "\n  ";
    output += "+ *";
    output += config->GetActivityRemarkLabel();
    output += ":*\n    ";
    output += FormatMultilineForList(record.activityRemark.value(), 4, " \\");
  }

  return output;
}

}  // namespace

namespace DayTypUtils {

void DisplayHeader(std::string& insights_stream, const DailyInsightsData& data,
                   const std::shared_ptr<DayTypConfig>& config) {
  insights_stream += TypUtils::BuildTitleText(
      config->GetTitleFont(), config->GetInsightsTitleFontSize(),
      config->GetSummarySectionLabel());
  insights_stream += "\n\n";

  insights_stream += BuildBulletLine(config->GetPeriodLabel(), data.date);
  insights_stream += "\n";
  insights_stream += BuildBulletLine(config->GetTotalTimeLabel(),
                                   TimeFormatDuration(data.total_duration));
  insights_stream += "\n";
  insights_stream += BuildBulletLine(config->GetActivityCountLabel(),
                                   std::to_string(data.activity_count));
  insights_stream += "\n";
  insights_stream +=
      BuildBulletLine(config->GetGetupTimeLabel(), data.metadata.getup_time);
  insights_stream += "\n";

  std::string formatted_remark =
      FormatMultilineForList(data.metadata.remark, 2, " \\");
  insights_stream += BuildBulletLine(config->GetRemarkLabel(), formatted_remark);
  insights_stream += "\n";
  if (!data.metadata.statuses.empty()) {
    insights_stream += "\n";
    insights_stream += TypUtils::BuildTitleText(
        config->GetCategoryTitleFont(), config->GetCategoryTitleFontSize(),
        config->GetCustomSectionLabel());
    insights_stream += "\n\n";
    for (const auto& status : data.metadata.statuses) {
      insights_stream += BuildBulletLine(
          status.label, FormatStatusStatistics(status.occurrence_count,
                                               status.total_duration,
                                               config->GetStatusCountUnit()));
      insights_stream += "\n";
    }
  }
}

void DisplayDetailedActivities(std::string& insights_stream,
                               const DailyInsightsData& data,
                               const std::shared_ptr<DayTypConfig>& config) {
  if (data.detailed_records.empty()) {
    return;
  }

  insights_stream += TypUtils::BuildTitleText(config->GetCategoryTitleFont(),
                                            config->GetCategoryTitleFontSize(),
                                            config->GetAllActivitiesLabel());
  insights_stream += "\n\n";

  for (const auto& record : data.detailed_records) {
    insights_stream += BuildActivityLine(record, config);
    insights_stream += "\n";
  }
}

}  // namespace DayTypUtils
