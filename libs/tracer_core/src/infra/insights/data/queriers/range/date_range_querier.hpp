// infra/insights/data/queriers/range/date_range_querier.hpp
#ifndef INFRASTRUCTURE_INSIGHTS_DATA_QUERIERS_RANGE_DATE_RANGE_QUERIER_H_
#define INFRASTRUCTURE_INSIGHTS_DATA_QUERIERS_RANGE_DATE_RANGE_QUERIER_H_

#include "infra/sqlite_fwd.hpp"

#include <string>
#include <string_view>

#include "domain/insights/models/period_insights_models.hpp"
#include "infra/insights/data/queriers/range_querier_base.hpp"

struct DateRangeQueryParam {
  std::string start_date;
  std::string end_date;
};

class DateRangeQuerier
    : public RangeQuerierBase<PeriodInsightsData, DateRangeQueryParam> {
 public:
  DateRangeQuerier(sqlite3* sqlite_db, std::string_view start_date,
                   std::string_view end_date);

 protected:
  [[nodiscard]] auto GetDateConditionSql() const -> std::string override;
  void BindSqlParameters(sqlite3_stmt* stmt) const override;
  [[nodiscard]] auto ValidateInput() const -> bool override;
  void HandleInvalidInput(PeriodInsightsData& data) const override;
  void PrepareData(PeriodInsightsData& data) const override;

 private:
  [[nodiscard]] auto TryBuildRequestedDays(int& requested_days) const -> bool;
};

#endif  // INFRASTRUCTURE_INSIGHTS_DATA_QUERIERS_RANGE_DATE_RANGE_QUERIER_H_
