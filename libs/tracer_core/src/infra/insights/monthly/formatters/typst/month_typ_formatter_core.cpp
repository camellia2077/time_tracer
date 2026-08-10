// infra/insights/monthly/formatters/typst/month_typ_formatter_core.cpp
#include <memory>
#include <string>

#include "infra/insights/monthly/formatters/typst/month_typ_formatter.hpp"
#include "infra/insights/shared/utils/format/insights_string_utils.hpp"
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

auto FormatRatio(int count, int total_days) -> std::string {
  return FormatCountWithPercentage(count, total_days);
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
  std::string title_text =
      FormatTitleTemplate(config_->GetTitleTemplate(), data);
  insights_stream += TypUtils::BuildTitleText(
      config_->GetTitleFont(), config_->GetInsightsTitleFontSize(), title_text);
  insights_stream += "\n\n";

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
    insights_stream +=
        BuildBulletLine(FormatBooleanCountLabel(config_->GetStatusDaysLabel(),
                                                data.status_true_days),
                        FormatRatio(data.status_true_days, data.actual_days));
    insights_stream += "\n";
    insights_stream +=
        BuildBulletLine(FormatBooleanCountLabel(config_->GetExerciseDaysLabel(),
                                                data.exercise_true_days),
                        FormatRatio(data.exercise_true_days, data.actual_days));
    insights_stream += "\n";
    insights_stream +=
        BuildBulletLine(FormatBooleanCountLabel(config_->GetCardioDaysLabel(),
                                                data.cardio_true_days),
                        FormatRatio(data.cardio_true_days, data.actual_days));
    insights_stream += "\n";
    insights_stream += BuildBulletLine(
        FormatBooleanCountLabel(config_->GetAnaerobicDaysLabel(),
                                data.anaerobic_true_days),
        FormatRatio(data.anaerobic_true_days, data.actual_days));
    insights_stream += "\n";
  }
}
