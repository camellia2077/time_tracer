// reports/monthly/queriers/month_querier.hpp
#ifndef REPORTS_MONTHLY_QUERIERS_MONTH_QUERIER_HPP_
#define REPORTS_MONTHLY_QUERIERS_MONTH_QUERIER_HPP_

#include <sqlite3.h>
#include <string>
#include "reports/shared/model/monthly_report_data.hpp"
#include "reports/shared/queriers/base_querier.hpp"

class MonthQuerier : public BaseQuerier<MonthlyReportData, const std::string&> {
public:
    explicit MonthQuerier(sqlite3* db, const std::string& year_month);

    // [FIX] Add the override for fetch_data
    MonthlyReportData fetch_data() override;

protected:
    std::string get_date_condition_sql() const override;
    void bind_sql_parameters(sqlite3_stmt* stmt) const override;
    bool _validate_input() const override;
    void _handle_invalid_input(MonthlyReportData& data) const override;
    void _prepare_data(MonthlyReportData& data) const override;
};

#endif // REPORTS_MONTHLY_QUERIERS_MONTH_QUERIER_HPP_