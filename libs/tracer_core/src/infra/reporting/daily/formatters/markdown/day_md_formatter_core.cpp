// infra/reporting/daily/formatters/markdown/day_md_formatter_core.cpp
#include <string>

#include "infra/reporting/daily/formatters/markdown/day_md_formatter.hpp"
#include "infra/reporting/shared/utils/format/report_string_utils.hpp"
#include "infra/reporting/shared/utils/format/time_format.hpp"

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

auto BuildActivityLine(const TimeRecord& record,
                       const std::string& project_path,
                       const std::string& end_only_time_format) -> std::string {
  std::string line;
  const std::string start_time = FormatClockTime(record.start_time);
  const std::string end_time = FormatClockTime(record.end_time);
  line.reserve(start_time.size() + end_time.size() +
               project_path.size() + kActivityLinePadding);
  line += "- ";
  if (record.kind == ActivityRecordKind::kEndOnly) {
    line += ReplaceAll(end_only_time_format, "{end_time}", end_time);
  } else {
    line += start_time;
    line += " - ";
    line += end_time;
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
}  // namespace

DayMdConfig::DayMdConfig(const DailyMdConfig& config)
    : DayBaseConfig(config.labels, {}),
      end_only_time_format_(config.end_only_time_format) {}

auto DayMdConfig::GetEndOnlyTimeFormat() const -> const std::string& {
  return end_only_time_format_;
}

DayMdFormatter::DayMdFormatter(std::shared_ptr<DayMdConfig> config)
    : BaseMdFormatter(std::move(config)) {}

auto DayMdFormatter::IsEmptyData(const DailyReportData& data) const -> bool {
  return data.activity_count == 0 && data.detailed_records.empty();
}

auto DayMdFormatter::GetAvgDays(const DailyReportData& /*data*/) const -> int {
  return 1;
}

auto DayMdFormatter::GetNoRecordsMsg() const -> std::string {
  return config_->GetNoRecords();
}

void DayMdFormatter::FormatHeaderContent(std::string& report_stream,
                                         const DailyReportData& data) const {
  report_stream += "## ";
  report_stream += config_->GetTitlePrefix();
  report_stream += " ";
  report_stream += data.date;
  report_stream += "\n\n";
  report_stream += BuildMarkdownItemLine(config_->GetDateLabel(), data.date);
  report_stream += BuildMarkdownItemLine(
      config_->GetTotalTimeLabel(), TimeFormatDuration(data.total_duration));
  report_stream += BuildMarkdownItemLine(
      config_->GetActivityCountLabel(), std::to_string(data.activity_count));
  for (const auto& status : data.metadata.statuses) {
    report_stream += BuildMarkdownItemLine(
        status.label, status.value ? "true" : "false");
  }
  report_stream += BuildMarkdownItemLine(config_->GetGetupTimeLabel(),
                                         data.metadata.getup_time);

  std::string formatted_remark = FormatMultilineForList(
      data.metadata.remark, kRemarkIndent, kRemarkIndentPrefix);
  report_stream +=
      BuildMarkdownItemLine(config_->GetRemarkLabel(), formatted_remark);
}

void DayMdFormatter::FormatExtraContent(std::string& report_stream,
                                        const DailyReportData& data) const {
  DisplayDetailedActivities(report_stream, data);
}

void DayMdFormatter::DisplayDetailedActivities(
    std::string& report_stream, const DailyReportData& data) const {
  if (data.detailed_records.empty()) {
    return;
  }

  report_stream += "\n## ";
  report_stream += config_->GetAllActivitiesLabel();
  report_stream += "\n\n";
  for (const auto& record : data.detailed_records) {
    std::string project_path =
        ReplaceAll(record.project_path, "_", config_->GetActivityConnector());
    report_stream += BuildActivityLine(record, project_path,
                                       config_->GetEndOnlyTimeFormat());
    if (record.activityRemark.has_value()) {
      // Keep the label on its own line so the layout stays readable for both
      // single-line and multiline remarks. <br> is intentional: a raw source
      // newline is collapsed by many Markdown renderers.
      report_stream += "  - **";
      report_stream += config_->GetActivityRemarkLabel();
      report_stream += "**:\n    ";
      report_stream +=
          FormatMultilineForList(record.activityRemark.value(), 4, "<br>");
      report_stream += "\n";
    }
  }
  report_stream += "\n";
}
