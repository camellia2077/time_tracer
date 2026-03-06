// infrastructure/reports/daily/formatters/typst/day_typ_formatter_core.cpp
#include <memory>
#include <string>

#include "infrastructure/reports/daily/formatters/statistics/stat_formatter.hpp"
#include "infrastructure/reports/daily/formatters/statistics/typst_strategy.hpp"
#include "infrastructure/reports/daily/formatters/typst/day_typ_formatter.hpp"
#include "infrastructure/reports/daily/formatters/typst/day_typ_utils.hpp"

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

auto DayTypFormatter::IsEmptyData(const DailyReportData& data) const -> bool {
  return data.total_duration == 0;
}

auto DayTypFormatter::GetAvgDays(const DailyReportData& /*data*/) const -> int {
  return 1;
}

auto DayTypFormatter::GetNoRecordsMsg() const -> std::string {
  return config_->GetNoRecords();
}

void DayTypFormatter::FormatHeaderContent(std::string& report_stream,
                                          const DailyReportData& data) const {
  DayTypUtils::DisplayHeader(report_stream, data, config_);
}

void DayTypFormatter::FormatExtraContent(std::string& report_stream,
                                         const DailyReportData& data) const {
  auto strategy = std::make_unique<TypstStrategy>(config_);
  StatFormatter stats_formatter(std::move(strategy));
  report_stream += stats_formatter.Format(data, config_);

  DayTypUtils::DisplayDetailedActivities(report_stream, data, config_);
}
