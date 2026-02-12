// infrastructure/reports/data/queriers/weekly/week_querier.hpp
#ifndef REPORTS_DATA_QUERIERS_WEEKLY_WEEK_QUERIER_H_
#define REPORTS_DATA_QUERIERS_WEEKLY_WEEK_QUERIER_H_

#include <sqlite3.h>

#include <string>

#include "domain/reports/models/weekly_report_data.hpp"
#include "infrastructure/reports/data/queriers/range_querier_base.hpp"
#include "infrastructure/reports/shared/utils/format/iso_week_utils.hpp"

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

#endif  // REPORTS_DATA_QUERIERS_WEEKLY_WEEK_QUERIER_H_
