#include "PeriodReportFormatter.h"
#include "report_generators/_shared/query_utils.h"
#include <iomanip>

std::string PeriodReportFormatter::format_report(const PeriodReportData& data, sqlite3* db) {
    std::stringstream ss;
    if (data.days_to_query <= 0) {
        ss << "Number of days to query must be positive.\n";
        return ss.str();
    }

    _display_summary(ss, data);

    if (data.actual_days == 0) {
        ss << "No time records found in this period.\n";
        return ss.str();
    }

    _display_project_breakdown(ss, data, db);
    return ss.str();
}

void PeriodReportFormatter::_display_summary(std::stringstream& ss, const PeriodReportData& data) {
    // Main title for the period report, including the date range
    ss << "## Period Report: Last " << data.days_to_query << " days ("
       << data.start_date << " to " << data.end_date << ")\n\n";

    // Display summary data only if there are records
    if (data.actual_days > 0) {
        ss << "- **Total Time Recorded**: " << time_format_duration(data.total_duration, data.actual_days) << "\n";
        ss << "- **Actual Days with Records**: " << data.actual_days << "\n";
    }
}

void PeriodReportFormatter::_display_project_breakdown(std::stringstream& ss, const PeriodReportData& data, sqlite3* db) {
    write_project_breakdown_to_stream(ss, db, data.records, data.total_duration, data.actual_days);
}