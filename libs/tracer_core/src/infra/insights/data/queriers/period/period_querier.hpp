// infra/insights/data/queriers/period/period_querier.hpp
#ifndef INFRASTRUCTURE_INSIGHTS_DATA_QUERIERS_PERIOD_PERIOD_QUERIER_H_
#define INFRASTRUCTURE_INSIGHTS_DATA_QUERIERS_PERIOD_PERIOD_QUERIER_H_

#include "infra/sqlite_fwd.hpp"

#include <string>

#include "application/ports/insights/i_platform_clock.hpp"
#include "domain/insights/models/period_insights_models.hpp"
#include "infra/insights/data/queriers/range_querier_base.hpp"

class PeriodQuerier : public RangeQuerierBase<PeriodInsightsData, int> {
 public:
  PeriodQuerier(
      sqlite3* sqlite_db, int days_to_query,
      const tracer_core::application::ports::IPlatformClock& platform_clock);

 protected:
  [[nodiscard]] auto GetDateConditionSql() const -> std::string override;
  void BindSqlParameters(sqlite3_stmt* stmt) const override;
  [[nodiscard]] auto ValidateInput() const -> bool override;
  void HandleInvalidInput(PeriodInsightsData& data) const override;
  void PrepareData(PeriodInsightsData& data) const override;

 private:
  const tracer_core::application::ports::IPlatformClock& platform_clock_;
  mutable std::string start_date_;
  mutable std::string end_date_;
};

#endif  // INFRASTRUCTURE_INSIGHTS_DATA_QUERIERS_PERIOD_PERIOD_QUERIER_H_
