// infra/insights/daily/formatters/typst/daily_typ_formatter_core.cpp
#include <memory>
#include <string>

#include "infra/insights/daily/formatters/statistics/stat_formatter.hpp"
#include "infra/insights/daily/formatters/statistics/typst_strategy.hpp"
#include "infra/insights/daily/formatters/typst/daily_typ_formatter.hpp"
#include "infra/insights/daily/formatters/typst/daily_typ_utils.hpp"

DailyTypFormatterConfig::DailyTypFormatterConfig(const DailyTypConfig& config)
    : DailyFormatterBaseConfig(config.labels, config.statistics_items),
      style_(config.fonts, config.layout),
      statistic_font_size_(config.statistic_font_size),
      statistic_title_font_size_(config.statistic_title_font_size),
      keyword_colors_(config.keyword_colors) {}

auto DailyTypFormatterConfig::GetStatisticFontSize() const -> int {
  return statistic_font_size_;
}

auto DailyTypFormatterConfig::GetStatisticTitleFontSize() const -> int {
  return statistic_title_font_size_;
}

auto DailyTypFormatterConfig::GetKeywordColors() const
    -> const std::map<std::string, std::string>& {
  return keyword_colors_;
}

DailyTypFormatter::DailyTypFormatter(
    std::shared_ptr<DailyTypFormatterConfig> config)
    : BaseTypFormatter(std::move(config)) {}

auto DailyTypFormatter::IsEmptyData(const DailyInsightsData& data) const
    -> bool {
  return data.activity.total_duration_seconds == 0;
}

auto DailyTypFormatter::GetAvgDays(const DailyInsightsData& /*data*/) const
    -> int {
  return 1;
}

auto DailyTypFormatter::GetNoRecordsMsg() const -> std::string {
  return config_->GetNoRecords();
}

void DailyTypFormatter::FormatHeaderContent(
    std::string& insights_stream, const DailyInsightsData& data) const {
  DailyTypUtils::DisplayHeader(insights_stream, data, config_);
}

void DailyTypFormatter::FormatExtraContent(
    std::string& insights_stream, const DailyInsightsData& data) const {
  auto strategy = std::make_unique<TypstStrategy>(config_);
  StatFormatter stats_formatter(std::move(strategy));
  insights_stream += stats_formatter.Format(data, config_);

  DailyTypUtils::DisplayDetailedActivities(insights_stream, data, config_);
}
