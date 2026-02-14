// infrastructure/reports/data/queriers/monthly/monthly_querier.hpp
#ifndef INFRASTRUCTURE_REPORTS_DATA_QUERIERS_MONTHLY_MONTHLY_QUERIER_H_
#define INFRASTRUCTURE_REPORTS_DATA_QUERIERS_MONTHLY_MONTHLY_QUERIER_H_

#include <sqlite3.h>

#include <map>
#include <string>

#include "domain/reports/models/period_report_models.hpp"
#include "infrastructure/reports/data/queriers/range_querier_base.hpp"

class MonthQuerier
    : public RangeQuerierBase<MonthlyReportData, std::string_view> {
 public:
  static constexpr int kYearMonthLength = 7;
  static constexpr int kDashPosition = 4;
  static constexpr int kYearEndPosition = 3;
  static constexpr int kMonthStartPosition = 5;
  static constexpr int kMonthEndPosition = 6;

  explicit MonthQuerier(sqlite3* sqlite_db, std::string_view year_month);

 protected:
  [[nodiscard]] auto GetDateConditionSql() const -> std::string override;
  void BindSqlParameters(sqlite3_stmt* stmt) const override;
  [[nodiscard]] auto ValidateInput() const -> bool override;
  void HandleInvalidInput(MonthlyReportData& data) const override;
  void PrepareData(MonthlyReportData& data) const override;
};

class BatchMonthDataFetcher {
 public:
  explicit BatchMonthDataFetcher(sqlite3* sqlite_db);

  [[nodiscard]] auto FetchAllData() -> std::map<std::string, MonthlyReportData>;

 private:
  sqlite3* db_;

  void FetchProjectStats(
      std::map<std::string, MonthlyReportData>& all_months_data,
      std::map<std::string, std::map<long long, long long>>& project_agg);
  void FetchActualDays(std::map<std::string, int>& actual_days);
  void FetchDayFlagCounts(std::map<std::string, int>& status_days,
                          std::map<std::string, int>& sleep_days,
                          std::map<std::string, int>& exercise_days,
                          std::map<std::string, int>& cardio_days,
                          std::map<std::string, int>& anaerobic_days);
};

#endif  // INFRASTRUCTURE_REPORTS_DATA_QUERIERS_MONTHLY_MONTHLY_QUERIER_H_
