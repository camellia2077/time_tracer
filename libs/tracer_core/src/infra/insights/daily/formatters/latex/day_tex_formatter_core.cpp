// infra/insights/daily/formatters/latex/day_tex_formatter_core.cpp
#include <memory>
#include <string>

#include "infra/insights/daily/formatters/latex/day_tex_formatter.hpp"
#include "infra/insights/daily/formatters/latex/day_tex_utils.hpp"
#include "infra/insights/daily/formatters/statistics/latex_strategy.hpp"
#include "infra/insights/daily/formatters/statistics/stat_formatter.hpp"

DayTexConfig::DayTexConfig(const DailyTexConfig& config)
    : DayBaseConfig(config.labels, config.statistics_items),
      style_(config.fonts, config.layout),
      keyword_colors_(config.keyword_colors) {}

auto DayTexConfig::GetKeywordColors() const
    -> const std::map<std::string, std::string>& {
  return keyword_colors_;
}

DayTexFormatter::DayTexFormatter(std::shared_ptr<DayTexConfig> config)
    : BaseTexFormatter(std::move(config)) {}

auto DayTexFormatter::IsEmptyData(const DailyInsightsData& data) const -> bool {
  return data.total_duration == 0;
}

auto DayTexFormatter::GetAvgDays(const DailyInsightsData& /*data*/) const -> int {
  return 1;
}

auto DayTexFormatter::GetNoRecordsMsg() const -> std::string {
  return config_->GetNoRecords();
}

auto DayTexFormatter::GetKeywordColors() const
    -> std::map<std::string, std::string> {
  return config_->GetKeywordColors();
}

void DayTexFormatter::FormatHeaderContent(std::string& insights_stream,
                                          const DailyInsightsData& data) const {
  DayTexUtils::DisplayHeader(insights_stream, data, config_);
}

void DayTexFormatter::FormatExtraContent(std::string& insights_stream,
                                         const DailyInsightsData& data) const {
  auto strategy = std::make_unique<LatexStrategy>(config_);
  StatFormatter stats_formatter(std::move(strategy));
  insights_stream += stats_formatter.Format(data, config_);
  DayTexUtils::DisplayDetailedActivities(insights_stream, data, config_);
}
