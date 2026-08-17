// infra/insights/monthly/formatters/markdown/month_md_formatter_core.cpp
#include <memory>
#include <string>

#include "infra/insights/monthly/formatters/markdown/month_md_formatter.hpp"
#include "infra/insights/shared/utils/format/insights_string_utils.hpp"
#include "infra/insights/shared/utils/format/status_statistics_format.hpp"
#include "infra/insights/shared/utils/format/time_format.hpp"

namespace {
constexpr size_t kMarkdownItemLinePadding = 8;

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
}  // namespace

MonthMdConfig::MonthMdConfig(const MonthlyMdConfig& config)
    : MonthBaseConfig(config.labels),
      project_breakdown_label_(config.labels.project_breakdown_label) {}

auto MonthMdConfig::GetProjectBreakdownLabel() const -> const std::string& {
  return project_breakdown_label_;
}

MonthMdFormatter::MonthMdFormatter(std::shared_ptr<MonthMdConfig> config)
    : BaseMdFormatter(std::move(config)) {}

auto MonthMdFormatter::ValidateData(const MonthlyInsightsData& data) const
    -> std::string {
  if (!data.is_valid) {
    return config_->GetInvalidFormatMessage();
  }
  return "";
}

auto MonthMdFormatter::IsEmptyData(const MonthlyInsightsData& data) const
    -> bool {
  return data.actual_days == 0;
}

auto MonthMdFormatter::GetAvgDays(const MonthlyInsightsData& data) const
    -> int {
  return data.actual_days;
}

auto MonthMdFormatter::GetNoRecordsMsg() const -> std::string {
  return config_->GetNoRecordsMessage();
}

void MonthMdFormatter::FormatHeaderContent(
    std::string& insights_stream, const MonthlyInsightsData& data) const {
  insights_stream += "## ";
  insights_stream += config_->GetSummarySectionLabel();
  insights_stream += "\n\n";

  insights_stream += BuildMarkdownItemLine(
      config_->GetPeriodLabel(), data.start_date + " - " + data.end_date);

  if (data.actual_days <= 0) {
    return;
  }

  insights_stream += BuildMarkdownItemLine(config_->GetActualDaysLabel(),
                                           std::to_string(data.actual_days));
  insights_stream += BuildMarkdownItemLine(
      config_->GetTotalTimeLabel(),
      TimeFormatDuration(data.activity.total_duration_seconds,
                         data.actual_days));
  insights_stream += BuildMarkdownItemLine(
      config_->GetActivityCountLabel(),
      FormatCountWithAverage(data.activity.occurrence_count,
                             data.requested_days));
  if (!data.statuses.empty()) {
    insights_stream += "\n## ";
    insights_stream += config_->GetCustomSectionLabel();
    insights_stream += "\n\n";
    for (const auto& status : data.statuses) {
      insights_stream += BuildMarkdownItemLine(
          status.label,
          FormatStatusStatistics(status.occurrence_count, status.total_duration,
                                 config_->GetStatusCountUnit()));
    }
  }
}
