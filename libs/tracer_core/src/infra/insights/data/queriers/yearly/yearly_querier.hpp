// infra/insights/data/queriers/yearly/yearly_querier.hpp
#ifndef INFRASTRUCTURE_INSIGHTS_DATA_QUERIERS_YEARLY_YEARLY_QUERIER_H_
#define INFRASTRUCTURE_INSIGHTS_DATA_QUERIERS_YEARLY_YEARLY_QUERIER_H_

#include "infra/sqlite_fwd.hpp"

#include <map>
#include <string>

#include "domain/insights/models/period_insights_models.hpp"
#include "infra/insights/data/queriers/range_querier_base.hpp"
#include "shared/utils/period_utils.hpp"

class YearQuerier
    : public RangeQuerierBase<YearlyInsightsData, std::string_view> {
 public:
  YearQuerier(sqlite3* sqlite_db, std::string_view year_str,
              const DailyStatusConfig* status_config = nullptr);
  [[nodiscard]] auto FetchData() -> YearlyInsightsData override;

 protected:
  [[nodiscard]] auto GetDateConditionSql() const -> std::string override;
  void BindSqlParameters(sqlite3_stmt* stmt) const override;

  [[nodiscard]] auto ValidateInput() const -> bool override;
  void HandleInvalidInput(YearlyInsightsData& data) const override;
  void PrepareData(YearlyInsightsData& data) const override;

 private:
  mutable std::string start_date_;
  mutable std::string end_date_;
  mutable int gregorian_year_ = 0;
};

class BatchYearDataFetcher {
 public:
  explicit BatchYearDataFetcher(sqlite3* sqlite_db);

  [[nodiscard]] auto FetchAllData() -> std::map<std::string, YearlyInsightsData>;

 private:
  sqlite3* db_;
};

#endif  // INFRASTRUCTURE_INSIGHTS_DATA_QUERIERS_YEARLY_YEARLY_QUERIER_H_
