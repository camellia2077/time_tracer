// infrastructure/reports/data/queriers/monthly/month_querier.hpp
#ifndef REPORTS_DATA_QUERIERS_MONTHLY_MONTH_QUERIER_H_
#define REPORTS_DATA_QUERIERS_MONTHLY_MONTH_QUERIER_H_

#include <sqlite3.h>

#include <string>

#include "domain/reports/models/monthly_report_data.hpp"
#include "infrastructure/reports/data/queriers/base_querier.hpp"

class MonthQuerier : public BaseQuerier<MonthlyReportData, std::string_view> {
 public:
  static constexpr int kYearMonthLength = 7;
  static constexpr int kDashPosition = 4;
  static constexpr int kYearEndPosition = 3;
  static constexpr int kMonthStartPosition = 5;
  static constexpr int kMonthEndPosition = 6;

  explicit MonthQuerier(sqlite3* sqlite_db, std::string_view year_month);

  [[nodiscard]] auto FetchData() -> MonthlyReportData override;

 protected:
  [[nodiscard]] auto GetDateConditionSql() const -> std::string override;
  void BindSqlParameters(sqlite3_stmt* stmt) const override;
  [[nodiscard]] auto ValidateInput() const -> bool override;
  void HandleInvalidInput(MonthlyReportData& data) const override;
  void PrepareData(MonthlyReportData& data) const override;
};

#endif  // REPORTS_DATA_QUERIERS_MONTHLY_MONTH_QUERIER_H_