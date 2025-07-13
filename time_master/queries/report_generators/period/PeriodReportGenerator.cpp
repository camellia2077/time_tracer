#include "PeriodReportGenerator.h"
#include "querier/PeriodReportQuerier.h"
#include "formatter/PeriodReportFormatter.h"
#include "report_generators/query_data_structs.h"

PeriodReportGenerator::PeriodReportGenerator(sqlite3* db) : m_db(db) {}

std::string PeriodReportGenerator::generate_report(int days) {
    // 1. Use the Querier to fetch data
    PeriodReportQuerier querier(m_db, days);
    PeriodReportData report_data = querier.fetch_data();

    // 2. Use the Formatter to create the report string
    PeriodReportFormatter formatter;
    return formatter.format_report(report_data, m_db);
}