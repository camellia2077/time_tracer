// queries/monthly/MonthQuerier.cpp
#include "MonthQuerier.hpp"
#include "queries/shared/utils/query_utils.hpp" 
#include <algorithm>
#include <cctype>

MonthQuerier::MonthQuerier(sqlite3* db, const std::string& year_month)
    : BaseQuerier(db, year_month) {}

MonthlyReportData MonthQuerier::fetch_data() {
    MonthlyReportData data = BaseQuerier::fetch_data();

    _fetch_actual_days(data);
    
    return data;
}

bool MonthQuerier::_validate_input() const {
    return m_param.length() == 6 && std::all_of(m_param.begin(), m_param.end(), ::isdigit);
}

void MonthQuerier::_handle_invalid_input(MonthlyReportData& data) const {
    data.year_month = "INVALID";
}

void MonthQuerier::_prepare_data(MonthlyReportData& data) const {
    data.year_month = m_param;
}

std::string MonthQuerier::get_date_condition_sql() const {
    return "SUBSTR(date, 1, 6) = ?";
}

void MonthQuerier::bind_sql_parameters(sqlite3_stmt* stmt) const {
    sqlite3_bind_text(stmt, 1, m_param.c_str(), -1, SQLITE_STATIC);
}