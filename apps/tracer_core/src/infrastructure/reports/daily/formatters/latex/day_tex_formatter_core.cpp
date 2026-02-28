// infrastructure/reports/daily/formatters/latex/day_tex_formatter_core.cpp
#include <memory>
#include <string>

#include "infrastructure/reports/daily/formatters/common/day_report_view_utils.hpp"
#include "infrastructure/reports/daily/formatters/latex/day_tex_formatter.hpp"
#include "infrastructure/reports/daily/formatters/latex/day_tex_utils.hpp"
#include "infrastructure/reports/daily/formatters/statistics/latex_strategy.hpp"
#include "infrastructure/reports/daily/formatters/statistics/stat_formatter.hpp"
#include "infrastructure/reports/shared/formatters/latex/tex_utils.hpp"
#include "infrastructure/reports/shared/interfaces/formatter_c_string_view_utils.hpp"

DayTexConfig::DayTexConfig(const TtDayTexConfigV1& config)
    : DayBaseConfig(config.labels, config.statisticsItems,
                    config.statisticsItemCount),
      style_(config.style) {
  report_title_ = formatter_c_string_view_utils::ToString(
      config.labels.reportTitle, "day.labels.reportTitle");
  keyword_colors_ = formatter_c_string_view_utils::BuildKeywordColorsMap(
      config.keywordColors, config.keywordColorCount, "day.keywordColors");
}

auto DayTexConfig::GetReportTitle() const -> const std::string& {
  return report_title_;
}

auto DayTexConfig::GetKeywordColors() const
    -> const std::map<std::string, std::string>& {
  return keyword_colors_;
}

DayTexFormatter::DayTexFormatter(std::shared_ptr<DayTexConfig> config)
    : BaseTexFormatter(std::move(config)) {}

auto DayTexFormatter::FormatReportFromView(
    const TtDailyReportDataV1& data_view) const -> std::string {
  DailyReportData data =
      day_report_view_utils::BuildDailyContentData(data_view);

  std::string output;
  output += GeneratePreamble();
  FormatHeaderContent(output, data);

  if (IsEmptyData(data)) {
    output += GetNoRecordsMsg();
    output += "\n";
    output += GeneratePostfix();
    return output;
  }

  FormatExtraContent(output, data);

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
      data_view.totalDuration, GetAvgDays(data),
      config_->GetCategoryTitleFontSize(), config_->GetListTopSepPt(),
      config_->GetListItemSepEx());
  output += GeneratePostfix();
  return output;
}

auto DayTexFormatter::IsEmptyData(const DailyReportData& data) const -> bool {
  return data.total_duration == 0;
}

auto DayTexFormatter::GetAvgDays(const DailyReportData& /*data*/) const -> int {
  return 1;
}

auto DayTexFormatter::GetNoRecordsMsg() const -> std::string {
  return config_->GetNoRecords();
}

auto DayTexFormatter::GetKeywordColors() const
    -> std::map<std::string, std::string> {
  return config_->GetKeywordColors();
}

void DayTexFormatter::FormatHeaderContent(std::string& report_stream,
                                          const DailyReportData& data) const {
  DayTexUtils::DisplayHeader(report_stream, data, config_);
}

void DayTexFormatter::FormatExtraContent(std::string& report_stream,
                                         const DailyReportData& data) const {
  auto strategy = std::make_unique<LatexStrategy>(config_);
  StatFormatter stats_formatter(std::move(strategy));
  report_stream += stats_formatter.Format(data, config_);
  DayTexUtils::DisplayDetailedActivities(report_stream, data, config_);
}
