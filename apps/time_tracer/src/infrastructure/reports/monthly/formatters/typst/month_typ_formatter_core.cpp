// infrastructure/reports/monthly/formatters/typst/month_typ_formatter_core.cpp
#include <memory>
#include <string>

#include "infrastructure/reports/monthly/formatters/typst/month_typ_formatter.hpp"
#include "infrastructure/reports/shared/formatters/typst/typ_utils.hpp"
#include "infrastructure/reports/shared/interfaces/range_report_view_utils.hpp"
#include "infrastructure/reports/shared/utils/format/report_string_utils.hpp"
#include "infrastructure/reports/shared/utils/format/time_format.hpp"

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

MonthTypConfig::MonthTypConfig(const TtMonthTypConfigV1& config)
    : MonthBaseConfig(config.labels),
      style_(config.style),
      margin_top_cm_(config.style.marginTopCm),
      margin_bottom_cm_(config.style.marginBottomCm),
      margin_left_cm_(config.style.marginLeftCm),
      margin_right_cm_(config.style.marginRightCm) {}

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

auto MonthTypFormatter::FormatReportFromView(
    const TtRangeReportDataV1& data_view) const -> std::string {
  auto summary_data =
      range_report_view_utils::BuildRangeLikeSummaryData<MonthlyReportData>(
          data_view);

  std::string report_stream;
  FormatPageSetup(report_stream);
  FormatTextSetup(report_stream);

  if (std::string error_message = ValidateData(summary_data);
      !error_message.empty()) {
    report_stream += error_message;
    report_stream += "\n";
    return report_stream;
  }

  FormatHeaderContent(report_stream, summary_data);

  if (IsEmptyData(summary_data)) {
    report_stream += GetNoRecordsMsg();
    report_stream += "\n";
    return report_stream;
  }

  report_stream += TypUtils::BuildTitleText(
      config_->GetCategoryTitleFont(), config_->GetCategoryTitleFontSize(),
      config_->GetProjectBreakdownLabel());
  report_stream += "\n\n";
  report_stream += TypUtils::FormatProjectTree(
      data_view.projectTreeNodes, data_view.projectTreeNodeCount,
      data_view.totalDuration, GetAvgDays(summary_data),
      config_->GetCategoryTitleFont(), config_->GetCategoryTitleFontSize());
  return report_stream;
}

auto MonthTypFormatter::ValidateData(const MonthlyReportData& data) const
    -> std::string {
  if (!data.is_valid) {
    return config_->GetInvalidFormatMessage();
  }
  return "";
}

auto MonthTypFormatter::IsEmptyData(const MonthlyReportData& data) const
    -> bool {
  return data.actual_days == 0;
}

auto MonthTypFormatter::GetAvgDays(const MonthlyReportData& data) const -> int {
  return data.actual_days;
}

auto MonthTypFormatter::GetNoRecordsMsg() const -> std::string {
  return config_->GetNoRecordsMessage();
}

void MonthTypFormatter::FormatPageSetup(std::string& report_stream) const {
  report_stream += TypUtils::BuildPageSetup(
      config_->GetMarginTopCm(), config_->GetMarginBottomCm(),
      config_->GetMarginLeftCm(), config_->GetMarginRightCm());
  report_stream += "\n";
}

void MonthTypFormatter::FormatHeaderContent(
    std::string& report_stream, const MonthlyReportData& data) const {
  std::string title_text =
      FormatTitleTemplate(config_->GetTitleTemplate(), data);
  report_stream += TypUtils::BuildTitleText(
      config_->GetTitleFont(), config_->GetReportTitleFontSize(), title_text);
  report_stream += "\n\n";

  if (data.actual_days > 0) {
    report_stream += BuildBulletLine(config_->GetActualDaysLabel(),
                                     std::to_string(data.actual_days));
    report_stream += "\n";
    report_stream += BuildBulletLine(
        config_->GetTotalTimeLabel(),
        TimeFormatDuration(data.total_duration, data.actual_days));
    report_stream += "\n";
    report_stream +=
        BuildBulletLine(config_->GetStatusDaysLabel(),
                        FormatRatio(data.status_true_days, data.actual_days));
    report_stream += "\n";
    report_stream +=
        BuildBulletLine(config_->GetSleepDaysLabel(),
                        FormatRatio(data.sleep_true_days, data.actual_days));
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
