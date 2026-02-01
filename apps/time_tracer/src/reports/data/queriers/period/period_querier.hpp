// reports/data/queriers/period/period_querier.hpp
#ifndef REPORTS_DATA_QUERIERS_PERIOD_PERIOD_QUERIER_H_
#define REPORTS_DATA_QUERIERS_PERIOD_PERIOD_QUERIER_H_

#include <sqlite3.h>

#include <string>

#include "reports/data/model/period_report_data.hpp"
#include "reports/data/queriers/base_querier.hpp"

class PeriodQuerier : public BaseQuerier<PeriodReportData, int> {
 public:
  explicit PeriodQuerier(sqlite3* sqlite_db, int days_to_query);

  [[nodiscard]] auto fetch_data() -> PeriodReportData override;

 protected:
  [[nodiscard]] auto get_date_condition_sql() const -> std::string override;
  void bind_sql_parameters(sqlite3_stmt* stmt) const override;
  [[nodiscard]] auto _validate_input() const -> bool override;
  void _handle_invalid_input(PeriodReportData& data) const override;
  void _prepare_data(PeriodReportData& data) const override;

 private:
  mutable std::string start_date_;
  mutable std::string end_date_;
};

#endif  // REPORTS_DATA_QUERIERS_PERIOD_PERIOD_QUERIER_H_