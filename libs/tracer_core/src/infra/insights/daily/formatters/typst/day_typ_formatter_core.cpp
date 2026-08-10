// infra/insights/daily/formatters/typst/day_typ_formatter_core.cpp
#include <memory>
#include <string>

#include "infra/insights/daily/formatters/statistics/stat_formatter.hpp"
#include "infra/insights/daily/formatters/statistics/typst_strategy.hpp"
#include "infra/insights/daily/formatters/typst/day_typ_formatter.hpp"
#include "infra/insights/daily/formatters/typst/day_typ_utils.hpp"

DayTypConfig::DayTypConfig(const DailyTypConfig& config)
    : DayBaseConfig(config.labels, config.statistics_items),
      style_(config.fonts, config.layout),
      statistic_font_size_(config.statistic_font_size),
      statistic_title_font_size_(config.statistic_title_font_size),
      keyword_colors_(config.keyword_colors) {}

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

DayTypFormatter::DayTypFormatter(std::shared_ptr<DayTypConfig> config)
    : BaseTypFormatter(std::move(config)) {}

auto DayTypFormatter::IsEmptyData(const DailyInsightsData& data) const -> bool {
  return data.total_duration == 0;
}

auto DayTypFormatter::GetAvgDays(const DailyInsightsData& /*data*/) const -> int {
  return 1;
}

auto DayTypFormatter::GetNoRecordsMsg() const -> std::string {
  return config_->GetNoRecords();
}

void DayTypFormatter::FormatHeaderContent(std::string& insights_stream,
                                          const DailyInsightsData& data) const {
  DayTypUtils::DisplayHeader(insights_stream, data, config_);
}

void DayTypFormatter::FormatExtraContent(std::string& insights_stream,
                                         const DailyInsightsData& data) const {
  auto strategy = std::make_unique<TypstStrategy>(config_);
  StatFormatter stats_formatter(std::move(strategy));
  insights_stream += stats_formatter.Format(data, config_);

  DayTypUtils::DisplayDetailedActivities(insights_stream, data, config_);
}
