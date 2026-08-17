// infra/insights/daily/formatters/latex/daily_tex_formatter_core.cpp
#include <memory>
#include <string>

#include "infra/insights/daily/formatters/latex/daily_tex_formatter.hpp"
#include "infra/insights/daily/formatters/latex/daily_tex_utils.hpp"
#include "infra/insights/daily/formatters/statistics/latex_strategy.hpp"
#include "infra/insights/daily/formatters/statistics/stat_formatter.hpp"

DailyTexFormatterConfig::DailyTexFormatterConfig(const DailyTexConfig& config)
    : DailyFormatterBaseConfig(config.labels, config.statistics_items),
      style_(config.fonts, config.layout),
      keyword_colors_(config.keyword_colors) {}

auto DailyTexFormatterConfig::GetKeywordColors() const
    -> const std::map<std::string, std::string>& {
  return keyword_colors_;
}

DailyTexFormatter::DailyTexFormatter(
    std::shared_ptr<DailyTexFormatterConfig> config)
    : BaseTexFormatter(std::move(config)) {}

auto DailyTexFormatter::IsEmptyData(const DailyInsightsData& data) const
    -> bool {
  return data.activity.total_duration_seconds == 0;
}

auto DailyTexFormatter::GetAvgDays(const DailyInsightsData& /*data*/) const
    -> int {
  return 1;
}

auto DailyTexFormatter::GetNoRecordsMsg() const -> std::string {
  return config_->GetNoRecords();
}

auto DailyTexFormatter::GetKeywordColors() const
    -> std::map<std::string, std::string> {
  return config_->GetKeywordColors();
}

void DailyTexFormatter::FormatHeaderContent(
    std::string& insights_stream, const DailyInsightsData& data) const {
  DailyTexUtils::DisplayHeader(insights_stream, data, config_);
}

void DailyTexFormatter::FormatExtraContent(
    std::string& insights_stream, const DailyInsightsData& data) const {
  auto strategy = std::make_unique<LatexStrategy>(config_);
  StatFormatter stats_formatter(std::move(strategy));
  insights_stream += stats_formatter.Format(data, config_);
  DailyTexUtils::DisplayDetailedActivities(insights_stream, data, config_);
}
