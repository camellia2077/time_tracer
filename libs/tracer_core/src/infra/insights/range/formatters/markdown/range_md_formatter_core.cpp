// infra/insights/range/formatters/markdown/range_md_formatter_core.cpp
#include <memory>
#include <string>

#include "infra/insights/range/formatters/markdown/range_md_formatter.hpp"
#include "infra/insights/shared/utils/format/insights_string_utils.hpp"
#include "infra/insights/shared/utils/format/status_statistics_format.hpp"
#include "infra/insights/shared/utils/format/time_format.hpp"

namespace {
constexpr std::size_t kMarkdownItemLineReservePadding = 8;

auto BuildMarkdownItemLine(const std::string& label, const std::string& value)
    -> std::string {
  std::string line;
  line.reserve(label.size() + value.size() + kMarkdownItemLineReservePadding);
  line += "- **";
  line += label;
  line += "**: ";
  line += value;
  line += "\n";
  return line;
}
}  // namespace

RangeMdConfig::RangeMdConfig(const RangeInsightsLabels& labels)
    : RangeBaseConfig(labels) {}

RangeMdFormatter::RangeMdFormatter(std::shared_ptr<RangeMdConfig> config)
    : BaseMdFormatter(std::move(config)) {}

auto RangeMdFormatter::ValidateData(const RangeInsightsData& data) const
    -> std::string {
  if (!data.is_valid) {
    return config_->GetInvalidRangeMessage();
  }
  return std::string{};
}

auto RangeMdFormatter::IsEmptyData(const RangeInsightsData& data) const
    -> bool {
  return data.actual_days == 0;
}

auto RangeMdFormatter::GetAvgDays(const RangeInsightsData& data) const -> int {
  return data.actual_days;
}

auto RangeMdFormatter::GetNoRecordsMsg() const -> std::string {
  return config_->GetNoRecordsMessage();
}

void RangeMdFormatter::FormatHeaderContent(
    std::string& insights_stream, const RangeInsightsData& data) const {
  insights_stream += "## ";
  insights_stream += config_->GetSummarySectionLabel();
  insights_stream += "\n\n";

  insights_stream += BuildMarkdownItemLine(
      config_->GetPeriodLabel(), data.start_date + " - " + data.end_date);

  if (data.actual_days <= 0) {
    return;
  }

  insights_stream += BuildMarkdownItemLine(
      config_->GetTotalTimeLabel(),
      TimeFormatDuration(data.activity.total_duration_seconds,
                         data.actual_days));
  insights_stream += BuildMarkdownItemLine(
      config_->GetActivityCountLabel(),
      FormatCountWithAverage(data.activity.occurrence_count,
                             data.requested_days));
  insights_stream += BuildMarkdownItemLine(config_->GetActualDaysLabel(),
                                           std::to_string(data.actual_days));
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
