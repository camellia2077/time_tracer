// reports/data/queriers/daily/batch_day_data_fetcher.hpp
#ifndef REPORTS_DATA_QUERIERS_DAILY_BATCH_DAY_DATA_FETCHER_H_
#define REPORTS_DATA_QUERIERS_DAILY_BATCH_DAY_DATA_FETCHER_H_

#include <sqlite3.h>

#include <map>
#include <string>
#include <tuple>
#include <vector>

#include "reports/data/interfaces/i_project_info_provider.hpp"
#include "reports/data/model/daily_report_data.hpp"

// 结构体定义保持不变 ...
struct BatchDataResult {
  std::map<std::string, DailyReportData> data_map;
  std::vector<std::tuple<std::string, int, int>> date_order;
};

class BatchDayDataFetcher {
 public:
  // [修改] 构造函数注入 provider
  explicit BatchDayDataFetcher(sqlite3* sqlite_db,
                               IProjectInfoProvider& provider);

  [[nodiscard]] auto fetch_all_data() -> BatchDataResult;

 private:
  sqlite3* db_;
  IProjectInfoProvider& provider_;  // [新增] 引用成员

  void fetch_days_metadata(BatchDataResult& result);
  void fetch_time_records(BatchDataResult& result);
};

#endif  // REPORTS_DATA_QUERIERS_DAILY_BATCH_DAY_DATA_FETCHER_H_