// application/compat/insights/i_insights_handler.hpp
#ifndef APPLICATION_COMPAT_INSIGHTS_I_INSIGHTS_HANDLER_H_
#define APPLICATION_COMPAT_INSIGHTS_I_INSIGHTS_HANDLER_H_

#include <string>
#include <string_view>
#include <vector>

#include "domain/insights/types/insights_types.hpp"

class IInsightsHandler {
 public:
  virtual ~IInsightsHandler() = default;

  virtual auto RunDailyQuery(std::string_view date, InsightsFormat format)
      -> std::string = 0;
  virtual auto RunMonthlyQuery(std::string_view month, InsightsFormat format)
      -> std::string = 0;
  virtual auto RunPeriodQuery(int days, InsightsFormat format)
      -> std::string = 0;
  virtual auto RunWeeklyQuery(std::string_view iso_week, InsightsFormat format)
      -> std::string = 0;
  virtual auto RunYearlyQuery(std::string_view year, InsightsFormat format)
      -> std::string = 0;
  virtual auto RunPeriodQueries(const std::vector<int>& days_list,
                                InsightsFormat format) -> std::string = 0;
};

#endif  // APPLICATION_COMPAT_INSIGHTS_I_INSIGHTS_HANDLER_H_
