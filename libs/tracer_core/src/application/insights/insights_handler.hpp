// application/insights/insights_handler.hpp
#ifndef APPLICATION_INSIGHTS_INSIGHTS_HANDLER_H_
#define APPLICATION_INSIGHTS_INSIGHTS_HANDLER_H_

#include <memory>

#include "application/compat/insights/i_insights_handler.hpp"

class IInsightsQueryService;

class InsightsHandler : public IInsightsHandler {
 public:
  explicit InsightsHandler(std::unique_ptr<IInsightsQueryService> query_service);
  ~InsightsHandler() override;

  auto RunDailyQuery(std::string_view date, InsightsFormat format)
      -> std::string override;
  auto RunMonthlyQuery(std::string_view month, InsightsFormat format)
      -> std::string override;
  auto RunPeriodQuery(int days, InsightsFormat format) -> std::string override;
  auto RunWeeklyQuery(std::string_view iso_week, InsightsFormat format)
      -> std::string override;
  auto RunYearlyQuery(std::string_view year, InsightsFormat format)
      -> std::string override;
  auto RunPeriodQueries(const std::vector<int>& days_list, InsightsFormat format)
      -> std::string override;

 private:
  std::unique_ptr<IInsightsQueryService> query_service_;
};

#endif  // APPLICATION_INSIGHTS_INSIGHTS_HANDLER_H_
