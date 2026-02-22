// infrastructure/reports/range/formatters/latex/range_tex_formatter_core.cpp
#include <memory>
#include <string>

#include "infrastructure/reports/range/formatters/latex/range_tex_formatter.hpp"
#include "infrastructure/reports/range/formatters/latex/range_tex_utils.hpp"
#include "infrastructure/reports/shared/formatters/latex/tex_utils.hpp"
#include "infrastructure/reports/shared/interfaces/range_report_view_utils.hpp"

namespace {}  // namespace

RangeTexConfig::RangeTexConfig(const TtRangeTexConfigV1& config)
    : RangeBaseConfig(config.labels), style_(config.style) {}

RangeTexFormatter::RangeTexFormatter(std::shared_ptr<RangeTexConfig> config)
    : BaseTexFormatter(std::move(config)) {}

auto RangeTexFormatter::FormatReportFromView(
    const TtRangeReportDataV1& data_view) const -> std::string {
  auto summary_data =
      range_report_view_utils::BuildRangeLikeSummaryData<RangeReportData>(
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
  const int kLineHeightTenths = kTitleSize * 12;
  const int kLineHeightScale = 10;
  output += "{";
  output += "\\fontsize{";
  output += std::to_string(kTitleSize);
  output += "}{";
  output += std::to_string(kLineHeightTenths / kLineHeightScale);
  output += ".";
  output += std::to_string(kLineHeightTenths % kLineHeightScale);
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

auto RangeTexFormatter::ValidateData(const RangeReportData& data) const
    -> std::string {
  if (!data.is_valid) {
    return config_->GetInvalidRangeMessage() + "\n";
  }
  return std::string{};
}

auto RangeTexFormatter::IsEmptyData(const RangeReportData& data) const -> bool {
  return data.actual_days == 0;
}

auto RangeTexFormatter::GetAvgDays(const RangeReportData& data) const -> int {
  return data.actual_days;
}

auto RangeTexFormatter::GetNoRecordsMsg() const -> std::string {
  return config_->GetNoRecordsMessage();
}

void RangeTexFormatter::FormatHeaderContent(std::string& report_stream,
                                            const RangeReportData& data) const {
  RangeTexUtils::DisplaySummary(report_stream, data, config_);
}
