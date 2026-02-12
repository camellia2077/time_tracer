// infrastructure/reports/daily/formatters/latex/day_tex_config.cpp
#include "infrastructure/reports/daily/formatters/latex/day_tex_config.hpp"

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
