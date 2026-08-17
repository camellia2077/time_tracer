// infra/insights/range/formatters/typst/range_typ_formatter_core.cpp
#include <memory>
#include <string>

#include "infra/insights/range/formatters/typst/range_typ_formatter.hpp"
#include "infra/insights/shared/utils/format/insights_string_utils.hpp"
#include "infra/insights/shared/utils/format/status_statistics_format.hpp"
#include "infra/insights/shared/utils/format/time_format.hpp"

namespace {
constexpr std::size_t kBulletLineReservePadding = 8;

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

}  // namespace

RangeTypConfig::RangeTypConfig(const RangeInsightsLabels& labels,
                               const FontConfig& fonts,
                               const LayoutConfig& layout)
    : RangeBaseConfig(labels),
      style_(fonts, layout),
      margin_top_cm_(layout.margin_top_cm),
      margin_bottom_cm_(layout.margin_bottom_cm),
      margin_left_cm_(layout.margin_left_cm),
      margin_right_cm_(layout.margin_right_cm) {}

auto RangeTypConfig::GetMarginTopCm() const -> double {
  return margin_top_cm_;
}

auto RangeTypConfig::GetMarginBottomCm() const -> double {
  return margin_bottom_cm_;
}

auto RangeTypConfig::GetMarginLeftCm() const -> double {
  return margin_left_cm_;
}

auto RangeTypConfig::GetMarginRightCm() const -> double {
  return margin_right_cm_;
}

RangeTypFormatter::RangeTypFormatter(std::shared_ptr<RangeTypConfig> config)
    : BaseTypFormatter(std::move(config)) {}

auto RangeTypFormatter::ValidateData(const RangeInsightsData& data) const
    -> std::string {
  if (!data.is_valid) {
    return config_->GetInvalidRangeMessage();
  }
  return std::string{};
}

auto RangeTypFormatter::IsEmptyData(const RangeInsightsData& data) const
    -> bool {
  return data.actual_days == 0;
}

auto RangeTypFormatter::GetAvgDays(const RangeInsightsData& data) const -> int {
  return data.actual_days;
}

auto RangeTypFormatter::GetNoRecordsMsg() const -> std::string {
  return config_->GetNoRecordsMessage();
}

void RangeTypFormatter::FormatPageSetup(std::string& insights_stream) const {
  insights_stream += TypUtils::BuildPageSetup(
      config_->GetMarginTopCm(), config_->GetMarginBottomCm(),
      config_->GetMarginLeftCm(), config_->GetMarginRightCm());
  insights_stream += "\n";
}

void RangeTypFormatter::FormatHeaderContent(
    std::string& insights_stream, const RangeInsightsData& data) const {
  insights_stream += TypUtils::BuildTitleText(
      config_->GetTitleFont(), config_->GetInsightsTitleFontSize(),
      config_->GetSummarySectionLabel());
  insights_stream += "\n\n";

  insights_stream += BuildBulletLine(config_->GetPeriodLabel(),
                                     data.start_date + " - " + data.end_date);
  insights_stream += "\n";

  if (data.actual_days > 0) {
    insights_stream +=
        BuildBulletLine(config_->GetTotalTimeLabel(),
                        TimeFormatDuration(data.activity.total_duration_seconds,
                                           data.actual_days));
    insights_stream += "\n";
    insights_stream +=
        BuildBulletLine(config_->GetActivityCountLabel(),
                        FormatCountWithAverage(data.activity.occurrence_count,
                                               data.requested_days));
    insights_stream += "\n";
    insights_stream += BuildBulletLine(config_->GetActualDaysLabel(),
                                       std::to_string(data.actual_days));
    insights_stream += "\n";
    if (!data.statuses.empty()) {
      insights_stream += "\n";
      insights_stream += TypUtils::BuildTitleText(
          config_->GetCategoryTitleFont(), config_->GetCategoryTitleFontSize(),
          config_->GetCustomSectionLabel());
      insights_stream += "\n\n";
      for (const auto& status : data.statuses) {
        insights_stream += BuildBulletLine(
            status.label, FormatStatusStatistics(
                              status.occurrence_count, status.total_duration,
                              config_->GetStatusCountUnit()));
        insights_stream += "\n";
      }
    }
  }
}
