// infrastructure/reports/data/queriers/yearly/yearly_querier.hpp
#ifndef INFRASTRUCTURE_REPORTS_DATA_QUERIERS_YEARLY_YEARLY_QUERIER_H_
#define INFRASTRUCTURE_REPORTS_DATA_QUERIERS_YEARLY_YEARLY_QUERIER_H_

#include <sqlite3.h>

#include <map>
#include <string>

#include "domain/reports/models/period_report_models.hpp"
#include "infrastructure/reports/data/queriers/range_querier_base.hpp"
#include "shared/utils/period_utils.hpp"

class YearQuerier
    : public RangeQuerierBase<YearlyReportData, std::string_view> {
 public:
  YearQuerier(sqlite3* sqlite_db, std::string_view year_str);

 protected:
  [[nodiscard]] auto GetDateConditionSql() const -> std::string override;
  void BindSqlParameters(sqlite3_stmt* stmt) const override;

  [[nodiscard]] auto ValidateInput() const -> bool override;
  void HandleInvalidInput(YearlyReportData& data) const override;
  void PrepareData(YearlyReportData& data) const override;

 private:
  mutable std::string start_date_;
  mutable std::string end_date_;
  mutable int gregorian_year_ = 0;
};

class BatchYearDataFetcher {
 public:
  explicit BatchYearDataFetcher(sqlite3* sqlite_db);

  [[nodiscard]] auto FetchAllData() -> std::map<std::string, YearlyReportData>;

 private:
  sqlite3* db_;
};

#endif  // INFRASTRUCTURE_REPORTS_DATA_QUERIERS_YEARLY_YEARLY_QUERIER_H_
