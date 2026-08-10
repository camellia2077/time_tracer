// infra/insights/range/formatters/markdown/range_md_formatter_core.cpp
#include <memory>
#include <string>

#include "infra/insights/range/formatters/markdown/range_md_formatter.hpp"
#include "infra/insights/shared/utils/format/insights_string_utils.hpp"
#include "infra/insights/shared/utils/format/time_format.hpp"

namespace {
constexpr std::size_t kMarkdownItemLineReservePadding = 8;

auto FormatRatio(int count, int total_days) -> std::string {
  return FormatCountWithPercentage(count, total_days);
}

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

auto RangeMdFormatter::IsEmptyData(const RangeInsightsData& data) const -> bool {
  return data.actual_days == 0;
}

auto RangeMdFormatter::GetAvgDays(const RangeInsightsData& data) const -> int {
  return data.actual_days;
}

auto RangeMdFormatter::GetNoRecordsMsg() const -> std::string {
  return config_->GetNoRecordsMessage();
}

void RangeMdFormatter::FormatHeaderContent(std::string& insights_stream,
                                           const RangeInsightsData& data) const {
  std::string title = FormatTitleTemplate(config_->GetTitleTemplate(), data);
  insights_stream += "## ";
  insights_stream += title;
  insights_stream += "\n\n";

  if (data.actual_days <= 0) {
    return;
  }

  insights_stream += BuildMarkdownItemLine(
      config_->GetTotalTimeLabel(),
      TimeFormatDuration(data.total_duration, data.actual_days));
  insights_stream += BuildMarkdownItemLine(
      config_->GetActivityCountLabel(),
      FormatCountWithAverage(data.matched_record_count, data.requested_days));
  insights_stream += BuildMarkdownItemLine(config_->GetActualDaysLabel(),
                                         std::to_string(data.actual_days));
  insights_stream += BuildMarkdownItemLine(
      FormatBooleanCountLabel(config_->GetStatusDaysLabel(),
                              data.status_true_days),
      FormatRatio(data.status_true_days, data.actual_days));
  insights_stream += BuildMarkdownItemLine(
      FormatBooleanCountLabel(config_->GetExerciseDaysLabel(),
                              data.exercise_true_days),
      FormatRatio(data.exercise_true_days, data.actual_days));
  insights_stream += BuildMarkdownItemLine(
      FormatBooleanCountLabel(config_->GetCardioDaysLabel(),
                              data.cardio_true_days),
      FormatRatio(data.cardio_true_days, data.actual_days));
  insights_stream += BuildMarkdownItemLine(
      FormatBooleanCountLabel(config_->GetAnaerobicDaysLabel(),
                              data.anaerobic_true_days),
      FormatRatio(data.anaerobic_true_days, data.actual_days));
}
