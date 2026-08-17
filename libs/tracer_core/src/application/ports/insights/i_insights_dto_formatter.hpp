// application/ports/insights/i_insights_dto_formatter.hpp
#ifndef APPLICATION_PORTS_I_INSIGHTS_DTO_FORMATTER_H_
#define APPLICATION_PORTS_I_INSIGHTS_DTO_FORMATTER_H_

#include <string>
#include <string_view>

#include "domain/insights/models/daily_insights_data.hpp"
#include "domain/insights/models/period_insights_models.hpp"
#include "domain/insights/types/insights_types.hpp"

namespace tracer_core::application::ports {

class IInsightsDtoFormatter {
 public:
  virtual ~IInsightsDtoFormatter() = default;

  virtual auto FormatDaily(const DailyInsightsData& insights,
                           InsightsFormat format) -> std::string = 0;
  virtual auto FormatMonthly(const MonthlyInsightsData& insights,
                             InsightsFormat format) -> std::string = 0;
  virtual auto FormatPeriod(const PeriodInsightsData& insights,
                            InsightsFormat format) -> std::string = 0;
  virtual auto FormatWeekly(const WeeklyInsightsData& insights,
                            InsightsFormat format) -> std::string = 0;
  virtual auto FormatYearly(const YearlyInsightsData& insights,
                            InsightsFormat format) -> std::string = 0;

  virtual auto FormatDailyLocalized(const DailyInsightsData& insights,
                                    InsightsFormat format,
                                    std::string_view locale) -> std::string {
    return FormatDaily(insights, format);
  }
  virtual auto FormatMonthlyLocalized(const MonthlyInsightsData& insights,
                                      InsightsFormat format,
                                      std::string_view locale) -> std::string {
    return FormatMonthly(insights, format);
  }
  virtual auto FormatPeriodLocalized(const PeriodInsightsData& insights,
                                     InsightsFormat format,
                                     std::string_view locale) -> std::string {
    return FormatPeriod(insights, format);
  }
  virtual auto FormatWeeklyLocalized(const WeeklyInsightsData& insights,
                                     InsightsFormat format,
                                     std::string_view locale) -> std::string {
    return FormatWeekly(insights, format);
  }
  virtual auto FormatYearlyLocalized(const YearlyInsightsData& insights,
                                     InsightsFormat format,
                                     std::string_view locale) -> std::string {
    return FormatYearly(insights, format);
  }
};

}  // namespace tracer_core::application::ports

#endif  // APPLICATION_PORTS_I_INSIGHTS_DTO_FORMATTER_H_
