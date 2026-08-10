// application/compat/insights/i_insights_query_service.hpp
#ifndef APPLICATION_COMPAT_INSIGHTS_I_INSIGHTS_QUERY_SERVICE_H_
#define APPLICATION_COMPAT_INSIGHTS_I_INSIGHTS_QUERY_SERVICE_H_

#include <string>
#include <string_view>
#include <vector>

#include "domain/insights/models/query_data_structs.hpp"
#include "domain/insights/types/insights_types.hpp"

class IInsightsQueryService {
 public:
  virtual ~IInsightsQueryService() = default;

  [[nodiscard]] virtual auto RunDailyQuery(std::string_view date_str,
                                           InsightsFormat format) const
      -> std::string = 0;
  [[nodiscard]] virtual auto RunPeriodQuery(int days, InsightsFormat format) const
      -> std::string = 0;
  [[nodiscard]] virtual auto RunMonthlyQuery(std::string_view year_month_str,
                                             InsightsFormat format) const
      -> std::string = 0;
  [[nodiscard]] virtual auto RunWeeklyQuery(std::string_view iso_week_str,
                                            InsightsFormat format) const
      -> std::string = 0;
  [[nodiscard]] virtual auto RunYearlyQuery(std::string_view year_str,
                                            InsightsFormat format) const
      -> std::string = 0;
};

#endif  // APPLICATION_COMPAT_INSIGHTS_I_INSIGHTS_QUERY_SERVICE_H_
