// infrastructure/reports/data/queriers/daily/daily_querier.hpp
#ifndef INFRASTRUCTURE_REPORTS_DATA_QUERIERS_DAILY_DAILY_QUERIER_H_
#define INFRASTRUCTURE_REPORTS_DATA_QUERIERS_DAILY_DAILY_QUERIER_H_

#include <sqlite3.h>

#include <map>
#include <string>
#include <tuple>
#include <vector>

#include "domain/reports/interfaces/i_project_info_provider.hpp"
#include "domain/reports/models/daily_report_data.hpp"
#include "infrastructure/reports/data/queriers/base_querier.hpp"

class DayQuerier : public BaseQuerier<DailyReportData, std::string_view> {
 public:
  explicit DayQuerier(sqlite3* sqlite_db, std::string_view date);

  [[nodiscard]] auto FetchData() -> DailyReportData override;

 protected:
  [[nodiscard]] auto GetDateConditionSql() const -> std::string override;
  void BindSqlParameters(sqlite3_stmt* stmt) const override;
  void PrepareData(DailyReportData& data) const override;

 private:
  void FetchMetadata(DailyReportData& data);
  void FetchDetailedRecords(DailyReportData& data);
  // --- [核心修改] 方法重命名，功能扩展 ---
  void FetchGeneratedStats(DailyReportData& data);
};

struct BatchDataResult {
  std::map<std::string, DailyReportData> data_map;
  std::vector<std::tuple<std::string, int, int>> date_order;
};

class BatchDayDataFetcher {
 public:
  explicit BatchDayDataFetcher(sqlite3* sqlite_db,
                               IProjectInfoProvider& provider);

  [[nodiscard]] auto FetchAllData() -> BatchDataResult;

 private:
  sqlite3* db_;
  IProjectInfoProvider& provider_;

  void FetchDaysMetadata(BatchDataResult& result);
  void FetchTimeRecords(BatchDataResult& result);
};

#endif  // INFRASTRUCTURE_REPORTS_DATA_QUERIERS_DAILY_DAILY_QUERIER_H_
