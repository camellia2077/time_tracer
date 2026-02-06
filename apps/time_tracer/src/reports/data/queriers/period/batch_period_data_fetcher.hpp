// reports/data/queriers/period/batch_period_data_fetcher.hpp
#ifndef REPORTS_DATA_QUERIERS_PERIOD_BATCH_PERIOD_DATA_FETCHER_H_
#define REPORTS_DATA_QUERIERS_PERIOD_BATCH_PERIOD_DATA_FETCHER_H_

#include <sqlite3.h>

#include <map>
#include <vector>

#include "reports/data/model/period_report_data.hpp"

/**
 * @class BatchPeriodDataFetcher
 * @brief 专用于批量获取周期报告数据。
 * * 优化策略：
 * 找出最大的周期天数，一次性从数据库获取该范围内的所有记录。
 * 然后在内存中为较短的周期进行过滤和聚合，避免重复查询数据库。
 */
class BatchPeriodDataFetcher {
 public:
  explicit BatchPeriodDataFetcher(sqlite3* db_connection);

  /**
   * @brief 为给定的天数列表获取聚合后的报告数据。
   * @param days_list 天数列表 (e.g., {7, 30, 90})
   * @return map<days, PeriodReportData>
   */
  auto FetchAllData(const std::vector<int>& days_list)
      -> std::map<int, PeriodReportData>;

 private:
  sqlite3* db_;

  // 内部结构，用于存储原始查询结果
  struct RawRecord {
    std::string date;
    long long project_id;
    long long duration;
  };
};

#endif  // REPORTS_DATA_QUERIERS_PERIOD_BATCH_PERIOD_DATA_FETCHER_H_