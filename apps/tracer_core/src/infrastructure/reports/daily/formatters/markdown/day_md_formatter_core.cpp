// infrastructure/reports/daily/formatters/markdown/day_md_formatter_core.cpp
#include <memory>
#include <string>

#include "infrastructure/reports/daily/formatters/common/day_report_view_utils.hpp"
#include "infrastructure/reports/daily/formatters/markdown/day_md_formatter.hpp"
#include "infrastructure/reports/daily/formatters/statistics/markdown_stat_strategy.hpp"
#include "infrastructure/reports/daily/formatters/statistics/stat_formatter.hpp"
#include "infrastructure/reports/shared/formatters/markdown/markdown_formatter.hpp"
#include "infrastructure/reports/shared/utils/format/report_string_utils.hpp"
#include "infrastructure/reports/shared/utils/format/time_format.hpp"

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
                       const std::string& project_path) -> std::string {
  std::string line;
  line.reserve(record.start_time.size() + record.end_time.size() +
               project_path.size() + kActivityLinePadding);
  line += "- ";
  line += record.start_time;
  line += " - ";
  line += record.end_time;
  line += " (";
  line += TimeFormatDuration(record.duration_seconds);
  line += "): ";
  line += project_path;
  line += "\n";
  return line;
}
}  // namespace

DayMdConfig::DayMdConfig(const TtDayMdConfigV1& config)
    : DayBaseConfig(config.labels, config.statisticsItems,
                    config.statisticsItemCount) {}

DayMdFormatter::DayMdFormatter(std::shared_ptr<DayMdConfig> config)
    : BaseMdFormatter(std::move(config)) {}

auto DayMdFormatter::FormatReportFromView(
    const TtDailyReportDataV1& data_view) const -> std::string {
  DailyReportData data =
      day_report_view_utils::BuildDailyContentData(data_view);

  std::string report_stream;
  FormatHeaderContent(report_stream, data);

  if (IsEmptyData(data)) {
    report_stream += GetNoRecordsMsg();
    report_stream += "\n";
    return report_stream;
  }

  FormatExtraContent(report_stream, data);
  report_stream += "\n## ";
  report_stream += config_->GetProjectBreakdownLabel();
  report_stream += "\n";
  report_stream += MarkdownFormatter::FormatProjectTree(
      data_view.projectTreeNodes, data_view.projectTreeNodeCount,
      data_view.totalDuration, GetAvgDays(data));
  return report_stream;
}

auto DayMdFormatter::IsEmptyData(const DailyReportData& data) const -> bool {
  return data.total_duration == 0;
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
  report_stream += BuildMarkdownItemLine(config_->GetStatusLabel(),
                                         BoolToString(data.metadata.status));
  report_stream += BuildMarkdownItemLine(config_->GetSleepLabel(),
                                         BoolToString(data.metadata.sleep));
  report_stream += BuildMarkdownItemLine(config_->GetExerciseLabel(),
                                         BoolToString(data.metadata.exercise));
  report_stream += BuildMarkdownItemLine(config_->GetGetupTimeLabel(),
                                         data.metadata.getup_time);

  std::string formatted_remark = FormatMultilineForList(
      data.metadata.remark, kRemarkIndent, kRemarkIndentPrefix);
  report_stream +=
      BuildMarkdownItemLine(config_->GetRemarkLabel(), formatted_remark);
}

void DayMdFormatter::FormatExtraContent(std::string& report_stream,
                                        const DailyReportData& data) const {
  auto strategy = std::make_unique<MarkdownStatStrategy>();
  StatFormatter stats_formatter(std::move(strategy));
  report_stream += stats_formatter.Format(data, config_);
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
    report_stream += BuildActivityLine(record, project_path);
    if (record.activityRemark.has_value()) {
      report_stream += "  - **";
      report_stream += config_->GetActivityRemarkLabel();
      report_stream += "**: ";
      report_stream += record.activityRemark.value();
      report_stream += "\n";
    }
  }
  report_stream += "\n";
}
