// queries/period/PeriodQuerier.hpp
#ifndef PERIOD_REPORT_QUERIER_HPP
#define PERIOD_REPORT_QUERIER_HPP

#include <sqlite3.h>
#include <string>
#include "queries/shared/data/PeriodReportData.hpp"
#include "queries/shared/queriers/BaseQuerier.hpp"

class PeriodQuerier : public BaseQuerier<PeriodReportData, int> {
public:
    explicit PeriodQuerier(sqlite3* db, int days_to_query);
    
    // [FIX] Overriding fetch_data to add the actual_days calculation
    PeriodReportData fetch_data() override;

protected:
    // [FIX] Corrected typo from std::string to std::string
    std::string get_date_condition_sql() const override;
    void bind_sql_parameters(sqlite3_stmt* stmt) const override;
    bool _validate_input() const override;
    void _handle_invalid_input(PeriodReportData& data) const override;
    void _prepare_data(PeriodReportData& data) const override;

private:
    mutable std::string start_date_;
    mutable std::string end_date_;
};

#endif // PERIOD_REPORT_QUERIER_HPP