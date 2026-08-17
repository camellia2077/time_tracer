// infra/insights/insights_dto_formatter.hpp
#ifndef INFRASTRUCTURE_INSIGHTS_INSIGHTS_DTO_FORMATTER_H_
#define INFRASTRUCTURE_INSIGHTS_INSIGHTS_DTO_FORMATTER_H_

#include <map>
#include <memory>
#include <string_view>

#include "application/ports/insights/i_insights_dto_formatter.hpp"
#include "infra/config/models/insights_catalog.hpp"
#include "infra/insights/shared/interfaces/i_insights_formatter.hpp"

namespace tracer::core::infrastructure::insights {
class InsightsDtoFormatter final
    : public tracer_core::application::ports::IInsightsDtoFormatter {
 public:
  explicit InsightsDtoFormatter(const InsightsCatalog& insights_catalog);

  auto FormatDaily(const DailyInsightsData& insights, InsightsFormat format)
      -> std::string override;
  auto FormatMonthly(const MonthlyInsightsData& insights, InsightsFormat format)
      -> std::string override;
  auto FormatPeriod(const PeriodInsightsData& insights, InsightsFormat format)
      -> std::string override;
  auto FormatWeekly(const WeeklyInsightsData& insights, InsightsFormat format)
      -> std::string override;
  auto FormatYearly(const YearlyInsightsData& insights, InsightsFormat format)
      -> std::string override;
  auto FormatDailyLocalized(const DailyInsightsData& insights,
                            InsightsFormat format, std::string_view locale)
      -> std::string override;
  auto FormatMonthlyLocalized(const MonthlyInsightsData& insights,
                              InsightsFormat format, std::string_view locale)
      -> std::string override;
  auto FormatPeriodLocalized(const PeriodInsightsData& insights,
                             InsightsFormat format, std::string_view locale)
      -> std::string override;
  auto FormatWeeklyLocalized(const WeeklyInsightsData& insights,
                             InsightsFormat format, std::string_view locale)
      -> std::string override;
  auto FormatYearlyLocalized(const YearlyInsightsData& insights,
                             InsightsFormat format, std::string_view locale)
      -> std::string override;

 private:
  template <typename InsightsDataType>
  auto FormatWithCache(
      const InsightsDataType& insights, InsightsFormat format,
      std::map<InsightsFormat,
               std::unique_ptr<IInsightsFormatter<InsightsDataType>>>& cache)
      -> std::string;

  const InsightsCatalog& insights_catalog_;
  std::map<InsightsFormat,
           std::unique_ptr<IInsightsFormatter<DailyInsightsData>>>
      daily_cache_;
  std::map<InsightsFormat,
           std::unique_ptr<IInsightsFormatter<MonthlyInsightsData>>>
      monthly_cache_;
  std::map<InsightsFormat,
           std::unique_ptr<IInsightsFormatter<PeriodInsightsData>>>
      period_cache_;
  std::map<InsightsFormat,
           std::unique_ptr<IInsightsFormatter<WeeklyInsightsData>>>
      weekly_cache_;
  std::map<InsightsFormat,
           std::unique_ptr<IInsightsFormatter<YearlyInsightsData>>>
      yearly_cache_;
};

}  // namespace tracer::core::infrastructure::insights

namespace infrastructure::insights {

using tracer::core::infrastructure::insights::InsightsDtoFormatter;

}  // namespace infrastructure::insights

#endif  // INFRASTRUCTURE_INSIGHTS_INSIGHTS_DTO_FORMATTER_H_
