// infra/insights/monthly/formatters/typst/month_typ_formatter_core.cpp
#include <memory>
#include <string>

#include "infra/insights/monthly/formatters/typst/month_typ_formatter.hpp"
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

MonthTypConfig::MonthTypConfig(const MonthlyTypConfig& config)
    : MonthBaseConfig(config.labels),
      style_(config.fonts, config.layout),
      margin_top_cm_(config.layout.margin_top_cm),
      margin_bottom_cm_(config.layout.margin_bottom_cm),
      margin_left_cm_(config.layout.margin_left_cm),
      margin_right_cm_(config.layout.margin_right_cm) {}

auto MonthTypConfig::GetMarginTopCm() const -> double {
  return margin_top_cm_;
}

auto MonthTypConfig::GetMarginBottomCm() const -> double {
  return margin_bottom_cm_;
}

auto MonthTypConfig::GetMarginLeftCm() const -> double {
  return margin_left_cm_;
}

auto MonthTypConfig::GetMarginRightCm() const -> double {
  return margin_right_cm_;
}

MonthTypFormatter::MonthTypFormatter(std::shared_ptr<MonthTypConfig> config)
    : BaseTypFormatter(std::move(config)) {}

auto MonthTypFormatter::ValidateData(const MonthlyInsightsData& data) const
    -> std::string {
  if (!data.is_valid) {
    return config_->GetInvalidFormatMessage();
  }
  return "";
}

auto MonthTypFormatter::IsEmptyData(const MonthlyInsightsData& data) const
    -> bool {
  return data.actual_days == 0;
}

auto MonthTypFormatter::GetAvgDays(const MonthlyInsightsData& data) const -> int {
  return data.actual_days;
}

auto MonthTypFormatter::GetNoRecordsMsg() const -> std::string {
  return config_->GetNoRecordsMessage();
}

void MonthTypFormatter::FormatPageSetup(std::string& insights_stream) const {
  insights_stream += TypUtils::BuildPageSetup(
      config_->GetMarginTopCm(), config_->GetMarginBottomCm(),
      config_->GetMarginLeftCm(), config_->GetMarginRightCm());
  insights_stream += "\n";
}

void MonthTypFormatter::FormatHeaderContent(
    std::string& insights_stream, const MonthlyInsightsData& data) const {
  insights_stream += TypUtils::BuildTitleText(
      config_->GetTitleFont(), config_->GetInsightsTitleFontSize(),
      config_->GetSummarySectionLabel());
  insights_stream += "\n\n";

  insights_stream += BuildBulletLine(config_->GetPeriodLabel(),
                                     data.start_date + " - " + data.end_date);
  insights_stream += "\n";

  if (data.actual_days > 0) {
    insights_stream += BuildBulletLine(config_->GetActualDaysLabel(),
                                     std::to_string(data.actual_days));
    insights_stream += "\n";
    insights_stream += BuildBulletLine(
        config_->GetTotalTimeLabel(),
        TimeFormatDuration(data.total_duration, data.actual_days));
    insights_stream += "\n";
    insights_stream += BuildBulletLine(
        config_->GetActivityCountLabel(),
        FormatCountWithAverage(data.matched_record_count, data.requested_days));
    insights_stream += "\n";
    if (!data.statuses.empty()) {
      insights_stream += "\n";
      insights_stream += TypUtils::BuildTitleText(
          config_->GetCategoryTitleFont(), config_->GetCategoryTitleFontSize(),
          config_->GetCustomSectionLabel());
      insights_stream += "\n\n";
      for (const auto& status : data.statuses) {
        insights_stream += BuildBulletLine(
            status.label, FormatStatusStatistics(status.occurrence_count,
                                                 status.total_duration,
                                                 config_->GetStatusCountUnit()));
        insights_stream += "\n";
      }
    }
  }
}
