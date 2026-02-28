// infrastructure/reports/daily/formatters/typst/day_typ_formatter_core.cpp
#include <memory>
#include <string>

#include "infrastructure/reports/daily/formatters/common/day_report_view_utils.hpp"
#include "infrastructure/reports/daily/formatters/statistics/stat_formatter.hpp"
#include "infrastructure/reports/daily/formatters/statistics/typst_strategy.hpp"
#include "infrastructure/reports/daily/formatters/typst/day_typ_formatter.hpp"
#include "infrastructure/reports/daily/formatters/typst/day_typ_utils.hpp"
#include "infrastructure/reports/shared/formatters/typst/typ_utils.hpp"
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

DayTypFormatter::DayTypFormatter(std::shared_ptr<DayTypConfig> config)
    : BaseTypFormatter(std::move(config)) {}

auto DayTypFormatter::FormatReportFromView(
    const TtDailyReportDataV1& data_view) const -> std::string {
  DailyReportData data =
      day_report_view_utils::BuildDailyContentData(data_view);

  std::string report_stream;
  FormatPageSetup(report_stream);
  FormatTextSetup(report_stream);
  FormatHeaderContent(report_stream, data);

  if (IsEmptyData(data)) {
    report_stream += GetNoRecordsMsg();
    report_stream += "\n";
    return report_stream;
  }

  FormatExtraContent(report_stream, data);
  report_stream += TypUtils::BuildTitleText(
      config_->GetCategoryTitleFont(), config_->GetCategoryTitleFontSize(),
      config_->GetProjectBreakdownLabel());
  report_stream += "\n\n";
  report_stream += TypUtils::FormatProjectTree(
      data_view.projectTreeNodes, data_view.projectTreeNodeCount,
      data_view.totalDuration, GetAvgDays(data),
      config_->GetCategoryTitleFont(), config_->GetCategoryTitleFontSize());
  return report_stream;
}

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
