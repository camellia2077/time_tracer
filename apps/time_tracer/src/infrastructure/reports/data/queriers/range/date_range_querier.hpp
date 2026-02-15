#ifndef INFRASTRUCTURE_REPORTS_DATA_QUERIERS_RANGE_DATE_RANGE_QUERIER_H_
#define INFRASTRUCTURE_REPORTS_DATA_QUERIERS_RANGE_DATE_RANGE_QUERIER_H_

#include <sqlite3.h>

#include <string>
#include <string_view>

#include "domain/reports/models/period_report_models.hpp"
#include "infrastructure/reports/data/queriers/range_querier_base.hpp"

struct DateRangeQueryParam {
  std::string start_date;
  std::string end_date;
};

class DateRangeQuerier
    : public RangeQuerierBase<PeriodReportData, DateRangeQueryParam> {
 public:
  DateRangeQuerier(sqlite3* sqlite_db, std::string_view start_date,
                   std::string_view end_date);

 protected:
  [[nodiscard]] auto GetDateConditionSql() const -> std::string override;
  void BindSqlParameters(sqlite3_stmt* stmt) const override;
  [[nodiscard]] auto ValidateInput() const -> bool override;
  void HandleInvalidInput(PeriodReportData& data) const override;
  void PrepareData(PeriodReportData& data) const override;

 private:
  [[nodiscard]] auto TryBuildRequestedDays(int& requested_days) const -> bool;
};

#endif  // INFRASTRUCTURE_REPORTS_DATA_QUERIERS_RANGE_DATE_RANGE_QUERIER_H_
