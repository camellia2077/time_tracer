// infrastructure/reports/data/queriers/weekly/batch_week_data_fetcher.hpp
#ifndef REPORTS_DATA_QUERIERS_WEEKLY_BATCH_WEEK_DATA_FETCHER_H_
#define REPORTS_DATA_QUERIERS_WEEKLY_BATCH_WEEK_DATA_FETCHER_H_

#include <sqlite3.h>

#include <map>
#include <string>

#include "domain/reports/models/weekly_report_data.hpp"

class BatchWeekDataFetcher {
 public:
  explicit BatchWeekDataFetcher(sqlite3* sqlite_db);

  [[nodiscard]] auto FetchAllData() -> std::map<std::string, WeeklyReportData>;

 private:
  sqlite3* db_;
};

#endif  // REPORTS_DATA_QUERIERS_WEEKLY_BATCH_WEEK_DATA_FETCHER_H_
