#include "DailyReportGenerator.h"

#include "querier/DailyReportQuerier.h"
#include "formatter/DailyReportFormatter.h"
#include "report_generators/query_data_structs.h"

DailyReportGenerator::DailyReportGenerator(sqlite3* db) : m_db(db) {}

std::string DailyReportGenerator::generate_report(const std::string& date) {
    // 1. Use the Querier module to fetch data
    DailyReportQuerier querier(m_db, date);
    DailyReportData report_data = querier.fetch_data();

    // 2. Use the Formatter module to create the report string
    DailyReportFormatter formatter;
    return formatter.format_report(report_data, m_db);
}