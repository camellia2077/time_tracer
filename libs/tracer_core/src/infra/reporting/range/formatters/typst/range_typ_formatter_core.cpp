// infra/reporting/range/formatters/typst/range_typ_formatter_core.cpp
#include <memory>
#include <string>

#include "infra/reporting/range/formatters/typst/range_typ_formatter.hpp"
#include "infra/reporting/shared/utils/format/report_string_utils.hpp"
#include "infra/reporting/shared/utils/format/time_format.hpp"

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

RangeTypConfig::RangeTypConfig(const RangeReportLabels& labels,
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

auto RangeTypFormatter::ValidateData(const RangeReportData& data) const
    -> std::string {
  if (!data.is_valid) {
    return config_->GetInvalidRangeMessage();
  }
  return std::string{};
}

auto RangeTypFormatter::IsEmptyData(const RangeReportData& data) const -> bool {
  return data.actual_days == 0;
}

auto RangeTypFormatter::GetAvgDays(const RangeReportData& data) const -> int {
  return data.actual_days;
}

auto RangeTypFormatter::GetNoRecordsMsg() const -> std::string {
  return config_->GetNoRecordsMessage();
}

void RangeTypFormatter::FormatPageSetup(std::string& report_stream) const {
  report_stream += TypUtils::BuildPageSetup(
      config_->GetMarginTopCm(), config_->GetMarginBottomCm(),
      config_->GetMarginLeftCm(), config_->GetMarginRightCm());
  report_stream += "\n";
}

void RangeTypFormatter::FormatHeaderContent(std::string& report_stream,
                                            const RangeReportData& data) const {
  std::string title = FormatTitleTemplate(config_->GetTitleTemplate(), data);
  report_stream += TypUtils::BuildTitleText(
      config_->GetTitleFont(), config_->GetReportTitleFontSize(), title);
  report_stream += "\n\n";

  if (data.actual_days > 0) {
    report_stream += BuildBulletLine(
        config_->GetTotalTimeLabel(),
        TimeFormatDuration(data.total_duration, data.actual_days));
    report_stream += "\n";
    report_stream += BuildBulletLine(config_->GetActualDaysLabel(),
                                     std::to_string(data.actual_days));
    report_stream += "\n";
    report_stream +=
        BuildBulletLine(config_->GetStatusDaysLabel(),
                        FormatRatio(data.status_true_days, data.actual_days));
    report_stream += "\n";
    report_stream += BuildBulletLine(
        config_->GetWakeAnchorDaysLabel(),
        FormatRatio(data.wake_anchor_true_days, data.actual_days));
    report_stream += "\n";
    report_stream +=
        BuildBulletLine(config_->GetExerciseDaysLabel(),
                        FormatRatio(data.exercise_true_days, data.actual_days));
    report_stream += "\n";
    report_stream +=
        BuildBulletLine(config_->GetCardioDaysLabel(),
                        FormatRatio(data.cardio_true_days, data.actual_days));
    report_stream += "\n";
    report_stream += BuildBulletLine(
        config_->GetAnaerobicDaysLabel(),
        FormatRatio(data.anaerobic_true_days, data.actual_days));
    report_stream += "\n";
  }
}
