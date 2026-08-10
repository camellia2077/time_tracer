// infra/insights/data/queriers/daily/daily_querier.hpp
#ifndef INFRASTRUCTURE_INSIGHTS_DATA_QUERIERS_DAILY_DAILY_QUERIER_H_
#define INFRASTRUCTURE_INSIGHTS_DATA_QUERIERS_DAILY_DAILY_QUERIER_H_

#include "infra/sqlite_fwd.hpp"

#include <map>
#include <string>
#include <tuple>
#include <vector>

#include "domain/insights/interfaces/i_project_info_provider.hpp"
#include "domain/insights/models/daily_insights_data.hpp"
#include "infra/config/models/insights_config_models.hpp"
#include "infra/insights/data/queriers/base_querier.hpp"

class DayQuerier : public BaseQuerier<DailyInsightsData, std::string_view> {
 public:
  explicit DayQuerier(sqlite3* sqlite_db, std::string_view date,
                      const DailyStatusConfig* status_config = nullptr);

  [[nodiscard]] auto FetchData() -> DailyInsightsData override;

 protected:
  [[nodiscard]] auto GetDateConditionSql() const -> std::string override;
  void BindSqlParameters(sqlite3_stmt* stmt) const override;
  void PrepareData(DailyInsightsData& data) const override;

 private:
  void FetchMetadata(DailyInsightsData& data);
  void FetchDetailedRecords(DailyInsightsData& data,
                            const IProjectInfoProvider& provider);
  const DailyStatusConfig* status_config_ = nullptr;
};

struct BatchDataResult {
  std::map<std::string, DailyInsightsData> data_map;
  std::vector<std::tuple<std::string, int, int>> date_order;
};

class BatchDayDataFetcher {
 public:
  explicit BatchDayDataFetcher(
      sqlite3* sqlite_db, IProjectInfoProvider& provider,
      const DailyStatusConfig* status_config = nullptr);

  [[nodiscard]] auto FetchAllData() -> BatchDataResult;

 private:
  sqlite3* db_;
  IProjectInfoProvider& provider_;
  const DailyStatusConfig* status_config_ = nullptr;

  void FetchDaysMetadata(BatchDataResult& result);
  void FetchTimeRecords(BatchDataResult& result);
};

#endif  // INFRASTRUCTURE_INSIGHTS_DATA_QUERIERS_DAILY_DAILY_QUERIER_H_
