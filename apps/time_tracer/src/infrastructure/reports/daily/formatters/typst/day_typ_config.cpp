// infrastructure/reports/daily/formatters/typst/day_typ_config.cpp
#include "infrastructure/reports/daily/formatters/typst/day_typ_config.hpp"

#include "infrastructure/reports/shared/interfaces/formatter_c_string_view_utils.hpp"

DayTypConfig::DayTypConfig(const TtDayTypConfigV1& config)
    : DayBaseConfig(config.labels, config.statisticsItems,
                    config.statisticsItemCount),
      style_(config.style),
      statistic_font_size_(config.statisticFontSize),
      statistic_title_font_size_(config.statisticTitleFontSize),
      keyword_colors_(formatter_c_string_view_utils::BuildKeywordColorsMap(
          config.keywordColors, config.keywordColorCount,
          "day.keywordColors")) {}

auto DayTypConfig::GetStatisticFontSize() const -> int {
  return statistic_font_size_;
}
auto DayTypConfig::GetStatisticTitleFontSize() const -> int {
  return statistic_title_font_size_;
}
auto DayTypConfig::GetKeywordColors() const
    -> const std::map<std::string, std::string>& {
  return keyword_colors_;
}
