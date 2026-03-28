// application/compat/reporting/i_report_query_service.hpp
#ifndef APPLICATION_COMPAT_REPORTING_I_REPORT_QUERY_SERVICE_H_
#define APPLICATION_COMPAT_REPORTING_I_REPORT_QUERY_SERVICE_H_

#include <string>
#include <string_view>
#include <vector>

#include "domain/reports/models/query_data_structs.hpp"
#include "domain/reports/types/report_types.hpp"

class IReportQueryService {
 public:
  virtual ~IReportQueryService() = default;

  [[nodiscard]] virtual auto RunDailyQuery(std::string_view date_str,
                                           ReportFormat format) const
      -> std::string = 0;
  [[nodiscard]] virtual auto RunPeriodQuery(int days, ReportFormat format) const
      -> std::string = 0;
  [[nodiscard]] virtual auto RunMonthlyQuery(std::string_view year_month_str,
                                             ReportFormat format) const
      -> std::string = 0;
  [[nodiscard]] virtual auto RunWeeklyQuery(std::string_view iso_week_str,
                                            ReportFormat format) const
      -> std::string = 0;
  [[nodiscard]] virtual auto RunYearlyQuery(std::string_view year_str,
                                            ReportFormat format) const
      -> std::string = 0;
};

#endif  // APPLICATION_COMPAT_REPORTING_I_REPORT_QUERY_SERVICE_H_
