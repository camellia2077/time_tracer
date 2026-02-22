// infrastructure/reports/monthly/formatters/latex/month_tex_formatter_core.cpp
#include <memory>
#include <string>

#include "infrastructure/reports/monthly/formatters/latex/month_tex_formatter.hpp"
#include "infrastructure/reports/monthly/formatters/latex/month_tex_utils.hpp"
#include "infrastructure/reports/shared/formatters/latex/tex_utils.hpp"
#include "infrastructure/reports/shared/interfaces/range_report_view_utils.hpp"

namespace {}  // namespace

MonthTexConfig::MonthTexConfig(const TtMonthTexConfigV1& config)
    : MonthBaseConfig(config.labels), style_(config.style) {}

MonthTexFormatter::MonthTexFormatter(std::shared_ptr<MonthTexConfig> config)
    : BaseTexFormatter(std::move(config)) {}

auto MonthTexFormatter::FormatReportFromView(
    const TtRangeReportDataV1& data_view) const -> std::string {
  auto summary_data =
      range_report_view_utils::BuildRangeLikeSummaryData<MonthlyReportData>(
          data_view);

  if (std::string error_message = ValidateData(summary_data);
      !error_message.empty()) {
    return error_message;
  }

  std::string output;
  output += GeneratePreamble();
  FormatHeaderContent(output, summary_data);

  if (IsEmptyData(summary_data)) {
    output += GetNoRecordsMsg();
    output += "\n";
    output += GeneratePostfix();
    return output;
  }

  const int kTitleSize = config_->GetCategoryTitleFontSize();
  constexpr int kLineHeightTenthsMultiplier = 12;
  constexpr int kLineHeightTenthsDivisor = 10;
  const int kLineHeightTenths = kTitleSize * kLineHeightTenthsMultiplier;
  output += "{";
  output += "\\fontsize{";
  output += std::to_string(kTitleSize);
  output += "}{";
  output += std::to_string(kLineHeightTenths / kLineHeightTenthsDivisor);
  output += ".";
  output += std::to_string(kLineHeightTenths % kLineHeightTenthsDivisor);
  output += "}\\selectfont";
  output += "\\section*{";
  output += TexUtils::EscapeLatex(config_->GetProjectBreakdownLabel());
  output += "}";
  output += "}\n\n";

  output += TexUtils::FormatProjectTree(
      data_view.projectTreeNodes, data_view.projectTreeNodeCount,
      data_view.totalDuration, GetAvgDays(summary_data),
      config_->GetCategoryTitleFontSize(), config_->GetListTopSepPt(),
      config_->GetListItemSepEx());
  output += GeneratePostfix();
  return output;
}

auto MonthTexFormatter::ValidateData(const MonthlyReportData& data) const
    -> std::string {
  if (!data.is_valid) {
    return config_->GetInvalidFormatMessage() + "\n";
  }
  return "";
}

auto MonthTexFormatter::IsEmptyData(const MonthlyReportData& data) const
    -> bool {
  return data.actual_days == 0;
}

auto MonthTexFormatter::GetAvgDays(const MonthlyReportData& data) const -> int {
  return data.actual_days;
}

auto MonthTexFormatter::GetNoRecordsMsg() const -> std::string {
  return config_->GetNoRecordsMessage();
}

void MonthTexFormatter::FormatHeaderContent(
    std::string& report_stream, const MonthlyReportData& data) const {
  MonthTexUtils::DisplayHeader(report_stream, data, config_);
}
