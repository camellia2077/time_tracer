// infra/insights/data/queriers/weekly/weekly_querier.hpp
#ifndef INFRASTRUCTURE_INSIGHTS_DATA_QUERIERS_WEEKLY_WEEKLY_QUERIER_H_
#define INFRASTRUCTURE_INSIGHTS_DATA_QUERIERS_WEEKLY_WEEKLY_QUERIER_H_

#include "infra/sqlite_fwd.hpp"

#include <map>
#include <string>

#include "domain/insights/models/period_insights_models.hpp"
#include "infra/insights/data/queriers/range_querier_base.hpp"
#include "shared/utils/period_utils.hpp"

class WeekQuerier
    : public RangeQuerierBase<WeeklyInsightsData, std::string_view> {
 public:
  WeekQuerier(sqlite3* sqlite_db, std::string_view iso_week);
  [[nodiscard]] auto FetchData() -> WeeklyInsightsData override;

 protected:
  [[nodiscard]] auto GetDateConditionSql() const -> std::string override;
  void BindSqlParameters(sqlite3_stmt* stmt) const override;

  [[nodiscard]] auto ValidateInput() const -> bool override;
  void HandleInvalidInput(WeeklyInsightsData& data) const override;
  void PrepareData(WeeklyInsightsData& data) const override;

 private:
  mutable std::string start_date_;
  mutable std::string end_date_;
  mutable IsoWeek parsed_week_;
};

class BatchWeekDataFetcher {
 public:
  explicit BatchWeekDataFetcher(sqlite3* sqlite_db);

  [[nodiscard]] auto FetchAllData() -> std::map<std::string, WeeklyInsightsData>;

 private:
  sqlite3* db_;
};

#endif  // INFRASTRUCTURE_INSIGHTS_DATA_QUERIERS_WEEKLY_WEEKLY_QUERIER_H_
