// infra/insights/data/queriers/monthly/monthly_querier.hpp
#ifndef INFRASTRUCTURE_INSIGHTS_DATA_QUERIERS_MONTHLY_MONTHLY_QUERIER_H_
#define INFRASTRUCTURE_INSIGHTS_DATA_QUERIERS_MONTHLY_MONTHLY_QUERIER_H_

#include "infra/sqlite_fwd.hpp"

#include <cstdint>
#include <map>
#include <string>

#include "domain/insights/interfaces/i_project_info_provider.hpp"
#include "domain/insights/models/period_insights_models.hpp"
#include "infra/insights/data/queriers/range_querier_base.hpp"

class MonthQuerier
    : public RangeQuerierBase<MonthlyInsightsData, std::string_view> {
 public:
  static constexpr int kYearMonthLength = 7;
  static constexpr int kDashPosition = 4;
  static constexpr int kYearEndPosition = 3;
  static constexpr int kMonthStartPosition = 5;
  static constexpr int kMonthEndPosition = 6;

  explicit MonthQuerier(sqlite3* sqlite_db, std::string_view year_month,
                        const DailyStatusConfig* status_config = nullptr);
  [[nodiscard]] auto FetchData() -> MonthlyInsightsData override;

 protected:
  [[nodiscard]] auto GetDateConditionSql() const -> std::string override;
  void BindSqlParameters(sqlite3_stmt* stmt) const override;
  [[nodiscard]] auto ValidateInput() const -> bool override;
  void HandleInvalidInput(MonthlyInsightsData& data) const override;
  void PrepareData(MonthlyInsightsData& data) const override;
};

class BatchMonthDataFetcher {
 public:
  explicit BatchMonthDataFetcher(sqlite3* sqlite_db);

  [[nodiscard]] auto FetchAllData()
      -> std::map<std::string, MonthlyInsightsData>;

 private:
  sqlite3* db_;

  void FetchProjectStats(
      std::map<std::string, MonthlyInsightsData>& all_months_data,
      std::map<std::string, std::map<std::int64_t, std::int64_t>>& project_agg,
      std::map<std::string, int>& status_days,
      std::map<std::string, int>& exercise_days,
      std::map<std::string, int>& cardio_days,
      std::map<std::string, int>& anaerobic_days,
      const IProjectInfoProvider& provider);
  void FetchActualDays(std::map<std::string, int>& actual_days);
};

#endif  // INFRASTRUCTURE_INSIGHTS_DATA_QUERIERS_MONTHLY_MONTHLY_QUERIER_H_
