// infrastructure/reports/data/queriers/period/period_querier.hpp
#ifndef REPORTS_DATA_QUERIERS_PERIOD_PERIOD_QUERIER_H_
#define REPORTS_DATA_QUERIERS_PERIOD_PERIOD_QUERIER_H_

#include <sqlite3.h>

#include <string>

#include "domain/reports/models/period_report_data.hpp"
#include "infrastructure/reports/data/queriers/range_querier_base.hpp"

class PeriodQuerier : public RangeQuerierBase<PeriodReportData, int> {
 public:
  explicit PeriodQuerier(sqlite3* sqlite_db, int days_to_query);

 protected:
  [[nodiscard]] auto GetDateConditionSql() const -> std::string override;
  void BindSqlParameters(sqlite3_stmt* stmt) const override;
  [[nodiscard]] auto ValidateInput() const -> bool override;
  void HandleInvalidInput(PeriodReportData& data) const override;
  void PrepareData(PeriodReportData& data) const override;

 private:
  mutable std::string start_date_;
  mutable std::string end_date_;
};

#endif  // REPORTS_DATA_QUERIERS_PERIOD_PERIOD_QUERIER_H_
