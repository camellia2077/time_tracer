#include "PeriodReportMarkdownFormatter.h"
#include "report_generators/_shared/query_utils.h"
#include <iomanip>

// [修改] 实现 PeriodReportMarkdownFormatter 类
std::string PeriodReportMarkdownFormatter::format_report(const PeriodReportData& data, sqlite3* db) const {
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

void PeriodReportMarkdownFormatter::_display_summary(std::stringstream& ss, const PeriodReportData& data) const {
    ss << "## Period Report: Last " << data.days_to_query << " days ("
       << data.start_date << " to " << data.end_date << ")\n\n";

    if (data.actual_days > 0) {
        ss << "- **Total Time Recorded**: " << time_format_duration(data.total_duration, data.actual_days) << "\n";
        ss << "- **Actual Days with Records**: " << data.actual_days << "\n";
    }
}

void PeriodReportMarkdownFormatter::_display_project_breakdown(std::stringstream& ss, const PeriodReportData& data, sqlite3* db) const {
    ProjectBreakdownFormatter formatter(db, data.records, data.total_duration, data.actual_days);
    formatter.write_to_stream(ss);
}