// reports/data/queriers/daily/day_querier.hpp
#ifndef REPORTS_DATA_QUERIERS_DAILY_DAY_QUERIER_H_
#define REPORTS_DATA_QUERIERS_DAILY_DAY_QUERIER_H_

#include <sqlite3.h>

#include <string>

#include "reports/data/model/daily_report_data.hpp"
#include "reports/data/queriers/base_querier.hpp"

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

#endif  // REPORTS_DATA_QUERIERS_DAILY_DAY_QUERIER_H_