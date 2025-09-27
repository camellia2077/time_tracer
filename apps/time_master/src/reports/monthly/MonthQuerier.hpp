// reports/monthly/MonthQuerier.hpp
#ifndef MONTH_QUERIER_HPP
#define MONTH_QUERIER_HPP

#include <sqlite3.h>
#include <string>
#include "reports/shared/data/MonthlyReportData.hpp"
#include "reports/shared/queriers/BaseQuerier.hpp"

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

#endif // MONTH_QUERIER_HPP