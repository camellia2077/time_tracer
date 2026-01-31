// reports/services/period_report_service.hpp
#ifndef REPORTS_SERVICES_PERIOD_REPORT_SERVICE_H_
#define REPORTS_SERVICES_PERIOD_REPORT_SERVICE_H_

#include <sqlite3.h>

#include <vector>

#include "common/config/app_config.hpp"
#include "reports/data/model/query_data_structs.hpp"
#include "reports/shared/types/report_format.hpp"

class PeriodReportService {
 public:
  explicit PeriodReportService(sqlite3* db, const AppConfig& config);

  /**
   * @brief 为指定的周期列表生成所有报告。
   * * 优化说明：
   * 采用了批量获取策略 (Batch Fetching)，对于包含多个周期（如 7, 30,
   * 90）的列表， 只会执行一次针对最大周期的数据库查询，然后内存过滤。
   */
  FormattedPeriodReports generate_reports(const std::vector<int>& days_list,
                                          ReportFormat format);

 private:
  sqlite3* db_;
  const AppConfig& app_config_;
};

#endif  // REPORTS_SERVICES_PERIOD_REPORT_SERVICE_H_
