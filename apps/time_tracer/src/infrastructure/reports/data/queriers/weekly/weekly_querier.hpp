// infrastructure/reports/data/queriers/weekly/weekly_querier.hpp
#ifndef INFRASTRUCTURE_REPORTS_DATA_QUERIERS_WEEKLY_WEEKLY_QUERIER_H_
#define INFRASTRUCTURE_REPORTS_DATA_QUERIERS_WEEKLY_WEEKLY_QUERIER_H_

#include <sqlite3.h>

#include <map>
#include <string>

#include "domain/reports/models/period_report_models.hpp"
#include "infrastructure/reports/data/queriers/range_querier_base.hpp"
#include "shared/utils/period_utils.hpp"

class WeekQuerier
    : public RangeQuerierBase<WeeklyReportData, std::string_view> {
 public:
  WeekQuerier(sqlite3* sqlite_db, std::string_view iso_week);

 protected:
  [[nodiscard]] auto GetDateConditionSql() const -> std::string override;
  void BindSqlParameters(sqlite3_stmt* stmt) const override;

  [[nodiscard]] auto ValidateInput() const -> bool override;
  void HandleInvalidInput(WeeklyReportData& data) const override;
  void PrepareData(WeeklyReportData& data) const override;

 private:
  mutable std::string start_date_;
  mutable std::string end_date_;
  mutable IsoWeek parsed_week_;
};

class BatchWeekDataFetcher {
 public:
  explicit BatchWeekDataFetcher(sqlite3* sqlite_db);

  [[nodiscard]] auto FetchAllData() -> std::map<std::string, WeeklyReportData>;

 private:
  sqlite3* db_;
};

#endif  // INFRASTRUCTURE_REPORTS_DATA_QUERIERS_WEEKLY_WEEKLY_QUERIER_H_
