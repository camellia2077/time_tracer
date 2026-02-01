// reports/data/queriers/daily/day_querier.hpp
#ifndef REPORTS_DATA_QUERIERS_DAILY_DAY_QUERIER_H_
#define REPORTS_DATA_QUERIERS_DAILY_DAY_QUERIER_H_

#include <sqlite3.h>

#include <string>

#include "reports/data/model/daily_report_data.hpp"
#include "reports/data/queriers/base_querier.hpp"

class DayQuerier : public BaseQuerier<DailyReportData, const std::string&> {
 public:
  explicit DayQuerier(sqlite3* sqlite_db, const std::string& date);

  [[nodiscard]] auto fetch_data() -> DailyReportData override;

 protected:
  [[nodiscard]] auto get_date_condition_sql() const -> std::string override;
  void bind_sql_parameters(sqlite3_stmt* stmt) const override;
  void _prepare_data(DailyReportData& data) const override;

 private:
  void _fetch_metadata(DailyReportData& data);
  void _fetch_detailed_records(DailyReportData& data);
  // --- [核心修改] 方法重命名，功能扩展 ---
  void _fetch_generated_stats(DailyReportData& data);
};

#endif  // REPORTS_DATA_QUERIERS_DAILY_DAY_QUERIER_H_