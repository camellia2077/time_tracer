// infrastructure/reports/range/formatters/markdown/range_md_formatter_core.cpp
#include <memory>
#include <string>

#include "infrastructure/reports/range/formatters/markdown/range_md_formatter.hpp"
#include "infrastructure/reports/shared/formatters/markdown/markdown_formatter.hpp"
#include "infrastructure/reports/shared/interfaces/range_report_view_utils.hpp"
#include "infrastructure/reports/shared/utils/format/report_string_utils.hpp"
#include "infrastructure/reports/shared/utils/format/time_format.hpp"

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

RangeMdConfig::RangeMdConfig(const TtRangeMdConfigV1& config)
    : RangeBaseConfig(config.labels) {}

RangeMdFormatter::RangeMdFormatter(std::shared_ptr<RangeMdConfig> config)
    : BaseMdFormatter(std::move(config)) {}

auto RangeMdFormatter::FormatReportFromView(
    const TtRangeReportDataV1& data_view) const -> std::string {
  RangeReportData summary_data =
      range_report_view_utils::BuildRangeLikeSummaryData<RangeReportData>(
          data_view);

  if (std::string error_message = ValidateData(summary_data);
      !error_message.empty()) {
    return error_message + "\n";
  }

  std::string report_stream;
  FormatHeaderContent(report_stream, summary_data);

  if (IsEmptyData(summary_data)) {
    report_stream += GetNoRecordsMsg();
    report_stream += "\n";
    return report_stream;
  }

  report_stream += "\n## ";
  report_stream += config_->GetProjectBreakdownLabel();
  report_stream += "\n";
  report_stream += MarkdownFormatter::FormatProjectTree(
      data_view.projectTreeNodes, data_view.projectTreeNodeCount,
      data_view.totalDuration, GetAvgDays(summary_data));
  return report_stream;
}

auto RangeMdFormatter::ValidateData(const RangeReportData& data) const
    -> std::string {
  if (!data.is_valid) {
    return config_->GetInvalidRangeMessage();
  }
  return std::string{};
}

auto RangeMdFormatter::IsEmptyData(const RangeReportData& data) const -> bool {
  return data.actual_days == 0;
}

auto RangeMdFormatter::GetAvgDays(const RangeReportData& data) const -> int {
  return data.actual_days;
}

auto RangeMdFormatter::GetNoRecordsMsg() const -> std::string {
  return config_->GetNoRecordsMessage();
}

void RangeMdFormatter::FormatHeaderContent(std::string& report_stream,
                                           const RangeReportData& data) const {
  std::string title = FormatTitleTemplate(config_->GetTitleTemplate(), data);
  report_stream += "## ";
  report_stream += title;
  report_stream += "\n\n";

  if (data.actual_days <= 0) {
    return;
  }

  report_stream += BuildMarkdownItemLine(
      config_->GetTotalTimeLabel(),
      TimeFormatDuration(data.total_duration, data.actual_days));
  report_stream += BuildMarkdownItemLine(config_->GetActualDaysLabel(),
                                         std::to_string(data.actual_days));
  report_stream += BuildMarkdownItemLine(
      config_->GetStatusDaysLabel(),
      FormatRatio(data.status_true_days, data.actual_days));
  report_stream += BuildMarkdownItemLine(
      config_->GetSleepDaysLabel(),
      FormatRatio(data.sleep_true_days, data.actual_days));
  report_stream += BuildMarkdownItemLine(
      config_->GetExerciseDaysLabel(),
      FormatRatio(data.exercise_true_days, data.actual_days));
  report_stream += BuildMarkdownItemLine(
      config_->GetCardioDaysLabel(),
      FormatRatio(data.cardio_true_days, data.actual_days));
  report_stream += BuildMarkdownItemLine(
      config_->GetAnaerobicDaysLabel(),
      FormatRatio(data.anaerobic_true_days, data.actual_days));
}
