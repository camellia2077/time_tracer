// infrastructure/reports/data/queriers/period/period_querier.hpp
#ifndef INFRASTRUCTURE_REPORTS_DATA_QUERIERS_PERIOD_PERIOD_QUERIER_H_
#define INFRASTRUCTURE_REPORTS_DATA_QUERIERS_PERIOD_PERIOD_QUERIER_H_

#include <sqlite3.h>

#include <string>

#include "application/ports/i_platform_clock.hpp"
#include "domain/reports/models/period_report_models.hpp"
#include "infrastructure/reports/data/queriers/range_querier_base.hpp"

class PeriodQuerier : public RangeQuerierBase<PeriodReportData, int> {
 public:
  PeriodQuerier(
      sqlite3* sqlite_db, int days_to_query,
      const time_tracer::application::ports::IPlatformClock& platform_clock);

 protected:
  [[nodiscard]] auto GetDateConditionSql() const -> std::string override;
  void BindSqlParameters(sqlite3_stmt* stmt) const override;
  [[nodiscard]] auto ValidateInput() const -> bool override;
  void HandleInvalidInput(PeriodReportData& data) const override;
  void PrepareData(PeriodReportData& data) const override;

 private:
  const time_tracer::application::ports::IPlatformClock& platform_clock_;
  mutable std::string start_date_;
  mutable std::string end_date_;
};

#endif  // INFRASTRUCTURE_REPORTS_DATA_QUERIERS_PERIOD_PERIOD_QUERIER_H_
