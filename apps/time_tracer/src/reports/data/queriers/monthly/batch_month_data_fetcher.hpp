// reports/data/queriers/monthly/batch_month_data_fetcher.hpp
#ifndef REPORTS_DATA_QUERIERS_MONTHLY_BATCH_MONTH_DATA_FETCHER_H_
#define REPORTS_DATA_QUERIERS_MONTHLY_BATCH_MONTH_DATA_FETCHER_H_

#include <sqlite3.h>

#include <map>
#include <string>

#include "reports/data/model/monthly_report_data.hpp"

/**
 * @class BatchMonthDataFetcher
 * @brief 专用于批量获取所有月份的报告数据。
 * * 职责：执行聚合 SQL 查询，解决 N+1 问题，将原始数据库行转换为
 * MonthlyReportData 对象。
 */
class BatchMonthDataFetcher {
 public:
  explicit BatchMonthDataFetcher(sqlite3* sqlite_db);

  /**
   * @brief 获取所有月份的数据。
   * @return map<"YYYY-MM", MonthlyReportData>
   */
  [[nodiscard]] auto FetchAllData() -> std::map<std::string, MonthlyReportData>;

 private:
  sqlite3* db_;

  // 内部辅助方法
  void FetchProjectStats(
      std::map<std::string, MonthlyReportData>& all_months_data,
      std::map<std::string, std::map<long long, long long>>& project_agg);
  void FetchActualDays(std::map<std::string, int>& actual_days);
};

#endif  // REPORTS_DATA_QUERIERS_MONTHLY_BATCH_MONTH_DATA_FETCHER_H_
