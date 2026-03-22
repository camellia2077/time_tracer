// infra/reports/daily/formatters/latex/day_tex_formatter_core.cpp
#include <memory>
#include <string>

#include "infra/reports/daily/formatters/latex/day_tex_formatter.hpp"
#include "infra/reports/daily/formatters/latex/day_tex_utils.hpp"
#include "infra/reports/daily/formatters/statistics/latex_strategy.hpp"
#include "infra/reports/daily/formatters/statistics/stat_formatter.hpp"

DayTexConfig::DayTexConfig(const DailyTexConfig& config)
    : DayBaseConfig(config.labels, config.statistics_items),
      style_(config.fonts, config.layout),
      report_title_(config.labels.report_title),
      keyword_colors_(config.keyword_colors) {}

auto DayTexConfig::GetReportTitle() const -> const std::string& {
  return report_title_;
}

auto DayTexConfig::GetKeywordColors() const
    -> const std::map<std::string, std::string>& {
  return keyword_colors_;
}

DayTexFormatter::DayTexFormatter(std::shared_ptr<DayTexConfig> config)
    : BaseTexFormatter(std::move(config)) {}

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
