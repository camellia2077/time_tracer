// infrastructure/reports/data/queriers/yearly/year_querier.hpp
#ifndef REPORTS_DATA_QUERIERS_YEARLY_YEAR_QUERIER_H_
#define REPORTS_DATA_QUERIERS_YEARLY_YEAR_QUERIER_H_

#include <sqlite3.h>

#include <string>

#include "domain/reports/models/yearly_report_data.hpp"
#include "infrastructure/reports/data/queriers/base_querier.hpp"
#include "infrastructure/reports/shared/utils/format/year_utils.hpp"

class YearQuerier : public BaseQuerier<YearlyReportData, std::string_view> {
 public:
  YearQuerier(sqlite3* sqlite_db, std::string_view year_str);

  [[nodiscard]] auto FetchData() -> YearlyReportData override;

 protected:
  [[nodiscard]] auto GetDateConditionSql() const -> std::string override;
  void BindSqlParameters(sqlite3_stmt* stmt) const override;

  [[nodiscard]] auto ValidateInput() const -> bool override;
  void HandleInvalidInput(YearlyReportData& data) const override;
  void PrepareData(YearlyReportData& data) const override;

 private:
  mutable std::string start_date_;
  mutable std::string end_date_;
  mutable int gregorian_year_ = 0;
};

#endif  // REPORTS_DATA_QUERIERS_YEARLY_YEAR_QUERIER_H_
