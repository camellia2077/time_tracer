// queries/period/PeriodQuerier.cpp
#include "PeriodQuerier.hpp"
#include "queries/shared/utils/query_utils.hpp"
#include <iomanip>

PeriodQuerier::PeriodQuerier(sqlite3* db, int days_to_query)
    : BaseQuerier(db, days_to_query) {}

// [FIX] Overriding fetch_data to add the call to _fetch_actual_days.
PeriodReportData PeriodQuerier::fetch_data() {
    // Call the base implementation to get records and total duration
    PeriodReportData data = BaseQuerier::fetch_data();

    // Now call the method specific to this querier type
    _fetch_actual_days(data);

    return data;
}

bool PeriodQuerier::_validate_input() const {
    return m_param > 0;
}

void PeriodQuerier::_handle_invalid_input(PeriodReportData& data) const {
    data.days_to_query = -1;
}

void PeriodQuerier::_prepare_data(PeriodReportData& data) const {
    m_end_date = get_current_date_str();
    m_start_date = add_days_to_date_str(m_end_date, -(m_param - 1));

    data.days_to_query = m_param;
    data.end_date = m_end_date;
    data.start_date = m_start_date;
}

std::string PeriodQuerier::get_date_condition_sql() const {
    return "date >= ? AND date <= ?";
}

void PeriodQuerier::bind_sql_parameters(sqlite3_stmt* stmt) const {
    sqlite3_bind_text(stmt, 1, m_start_date.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, m_end_date.c_str(), -1, SQLITE_TRANSIENT);
}