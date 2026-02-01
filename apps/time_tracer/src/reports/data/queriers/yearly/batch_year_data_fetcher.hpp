// reports/data/queriers/yearly/batch_year_data_fetcher.hpp
#ifndef REPORTS_DATA_QUERIERS_YEARLY_BATCH_YEAR_DATA_FETCHER_H_
#define REPORTS_DATA_QUERIERS_YEARLY_BATCH_YEAR_DATA_FETCHER_H_

#include <sqlite3.h>

#include <map>
#include <string>

#include "reports/data/model/yearly_report_data.hpp"

class BatchYearDataFetcher {
 public:
  explicit BatchYearDataFetcher(sqlite3* sqlite_db);

  std::map<std::string, YearlyReportData> fetch_all_data();

 private:
  sqlite3* db_;
};

#endif  // REPORTS_DATA_QUERIERS_YEARLY_BATCH_YEAR_DATA_FETCHER_H_
