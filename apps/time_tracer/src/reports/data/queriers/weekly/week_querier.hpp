// reports/data/queriers/weekly/week_querier.hpp
#ifndef REPORTS_DATA_QUERIERS_WEEKLY_WEEK_QUERIER_H_
#define REPORTS_DATA_QUERIERS_WEEKLY_WEEK_QUERIER_H_

#include <sqlite3.h>

#include <string>

#include "reports/data/model/weekly_report_data.hpp"
#include "reports/data/queriers/base_querier.hpp"
#include "reports/shared/utils/format/iso_week_utils.hpp"

class WeekQuerier : public BaseQuerier<WeeklyReportData, std::string> {
 public:
  WeekQuerier(sqlite3* sqlite_db, const std::string& iso_week);

  WeeklyReportData fetch_data() override;

 protected:
  std::string get_date_condition_sql() const override;
  void bind_sql_parameters(sqlite3_stmt* stmt) const override;

  bool _validate_input() const override;
  void _handle_invalid_input(WeeklyReportData& data) const override;
  void _prepare_data(WeeklyReportData& data) const override;

 private:
  mutable std::string start_date_;
  mutable std::string end_date_;
  mutable IsoWeek parsed_week_;
};

#endif  // REPORTS_DATA_QUERIERS_WEEKLY_WEEK_QUERIER_H_
