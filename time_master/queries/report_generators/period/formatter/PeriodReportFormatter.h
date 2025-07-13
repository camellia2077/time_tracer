#ifndef PERIOD_REPORT_FORMATTER_H
#define PERIOD_REPORT_FORMATTER_H

#include <sqlite3.h>
#include <string>
#include <sstream>
#include "report_generators/query_data_structs.h"

class PeriodReportFormatter {
public:
    std::string format_report(const PeriodReportData& data, sqlite3* db);

private:
    void _display_summary(std::stringstream& ss, const PeriodReportData& data);
    void _display_project_breakdown(std::stringstream& ss, const PeriodReportData& data, sqlite3* db);
};

#endif // PERIOD_REPORT_FORMATTER_H