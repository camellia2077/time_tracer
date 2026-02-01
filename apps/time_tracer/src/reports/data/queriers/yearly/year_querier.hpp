// reports/data/queriers/yearly/year_querier.hpp
#ifndef REPORTS_DATA_QUERIERS_YEARLY_YEAR_QUERIER_H_
#define REPORTS_DATA_QUERIERS_YEARLY_YEAR_QUERIER_H_

#include <sqlite3.h>

#include <string>

#include "reports/data/model/yearly_report_data.hpp"
#include "reports/data/queriers/base_querier.hpp"
#include "reports/shared/utils/format/year_utils.hpp"

class YearQuerier : public BaseQuerier<YearlyReportData, std::string> {
 public:
  YearQuerier(sqlite3* sqlite_db, const std::string& year_str);

  YearlyReportData fetch_data() override;

 protected:
  std::string get_date_condition_sql() const override;
  void bind_sql_parameters(sqlite3_stmt* stmt) const override;

  bool _validate_input() const override;
  void _handle_invalid_input(YearlyReportData& data) const override;
  void _prepare_data(YearlyReportData& data) const override;

 private:
  mutable std::string start_date_;
  mutable std::string end_date_;
  mutable int gregorian_year_ = 0;
};

#endif  // REPORTS_DATA_QUERIERS_YEARLY_YEAR_QUERIER_H_
