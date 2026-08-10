// infra/insights/daily/formatters/markdown/day_md_formatter_core.cpp
#include <string>

#include "infra/insights/daily/formatters/markdown/day_md_formatter.hpp"
#include "infra/insights/shared/utils/format/insights_string_utils.hpp"
#include "infra/insights/shared/utils/format/time_format.hpp"

namespace {
constexpr int kRemarkIndent = 2;
const std::string kRemarkIndentPrefix = "  ";
constexpr size_t kMarkdownItemLinePadding = 8;
constexpr size_t kActivityLinePadding = 32;

auto BuildMarkdownItemLine(const std::string& label, const std::string& value)
    -> std::string {
  std::string line;
  line.reserve(label.size() + value.size() + kMarkdownItemLinePadding);
  line += "- **";
  line += label;
  line += "**: ";
  line += value;
  line += "\n";
  return line;
}

// NOLINTBEGIN(bugprone-easily-swappable-parameters)
auto BuildActivityLine(const TimeRecord& record,
                       const std::string& project_path,
                       const std::string& end_only_time_format) -> std::string {
  std::string line;
  const std::string kStartTime = FormatClockTime(record.start_time);
  const std::string kEndTime = FormatClockTime(record.end_time);
  line.reserve(kStartTime.size() + kEndTime.size() + project_path.size() +
               kActivityLinePadding);
  line += "- ";
  if (record.kind == ActivityRecordKind::kEndOnly) {
    line += ReplaceAll(end_only_time_format, "{end_time}", kEndTime);
  } else {
    line += kStartTime;
    line += " - ";
    line += kEndTime;
    line += " (";
    line += TimeFormatDuration(record.duration_seconds);
    line += "): ";
  }
  if (record.kind == ActivityRecordKind::kEndOnly) {
    line += ": ";
  }
  line += project_path;
  line += "\n";
  return line;
}
// NOLINTEND(bugprone-easily-swappable-parameters)
}  // namespace

DayMdConfig::DayMdConfig(const DailyMdConfig& config)
    : DayBaseConfig(config.labels, {}),
      end_only_time_format_(config.end_only_time_format) {}

auto DayMdConfig::GetEndOnlyTimeFormat() const -> const std::string& {
  return end_only_time_format_;
}

DayMdFormatter::DayMdFormatter(std::shared_ptr<DayMdConfig> config)
    : BaseMdFormatter(std::move(config)) {}

auto DayMdFormatter::IsEmptyData(const DailyInsightsData& data) const -> bool {
  return data.activity_count == 0 && data.detailed_records.empty();
}

auto DayMdFormatter::GetAvgDays(const DailyInsightsData& /*data*/) const -> int {
  return 1;
}

auto DayMdFormatter::GetNoRecordsMsg() const -> std::string {
  return config_->GetNoRecords();
}

void DayMdFormatter::FormatHeaderContent(std::string& insights_stream,
                                         const DailyInsightsData& data) const {
  insights_stream += "## ";
  insights_stream += config_->GetTitlePrefix();
  insights_stream += " ";
  insights_stream += data.date;
  insights_stream += "\n\n";
  insights_stream += BuildMarkdownItemLine(config_->GetDateLabel(), data.date);
  insights_stream += BuildMarkdownItemLine(
      config_->GetTotalTimeLabel(), TimeFormatDuration(data.total_duration));
  insights_stream += BuildMarkdownItemLine(config_->GetActivityCountLabel(),
                                         std::to_string(data.activity_count));
  for (const auto& status : data.metadata.statuses) {
    insights_stream +=
        BuildMarkdownItemLine(status.label, status.value ? "true" : "false");
  }
  insights_stream += BuildMarkdownItemLine(config_->GetGetupTimeLabel(),
                                         data.metadata.getup_time);

  std::string formatted_remark = FormatMultilineForList(
      data.metadata.remark, kRemarkIndent, kRemarkIndentPrefix);
  insights_stream +=
      BuildMarkdownItemLine(config_->GetRemarkLabel(), formatted_remark);
}

void DayMdFormatter::FormatExtraContent(std::string& insights_stream,
                                        const DailyInsightsData& data) const {
  DisplayDetailedActivities(insights_stream, data);
}

void DayMdFormatter::DisplayDetailedActivities(
    std::string& insights_stream, const DailyInsightsData& data) const {
  if (data.detailed_records.empty()) {
    return;
  }

  insights_stream += "\n## ";
  insights_stream += config_->GetAllActivitiesLabel();
  insights_stream += "\n\n";
  for (const auto& record : data.detailed_records) {
    std::string project_path =
        ReplaceAll(record.project_path, "_", config_->GetActivityConnector());
    insights_stream += BuildActivityLine(record, project_path,
                                       config_->GetEndOnlyTimeFormat());
    if (record.activityRemark.has_value()) {
      // Keep the label on its own line so the layout stays readable for both
      // single-line and multiline remarks. <br> is intentional: a raw source
      // newline is collapsed by many Markdown renderers.
      insights_stream += "  - **";
      insights_stream += config_->GetActivityRemarkLabel();
      insights_stream += "**:\n    ";
      insights_stream +=
          FormatMultilineForList(record.activityRemark.value(), 4, "<br>");
      insights_stream += "\n";
    }
  }
  insights_stream += "\n";
}
