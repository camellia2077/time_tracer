#include "MonthlyReportMarkdownFormatter.h"
#include "report_generators/_shared/query_utils.h"
#include <iomanip>

// [修改] 实现 MonthlyReportMarkdownFormatter 类
std::string MonthlyReportMarkdownFormatter::format_report(const MonthlyReportData& data, sqlite3* db) const {
    std::stringstream ss;
    if (data.year_month == "INVALID") {
        ss << "Invalid year_month format. Expected YYYYMM.\n";
        return ss.str();
    }

    _display_summary(ss, data);

    if (data.actual_days == 0) {
        ss << "No time records found for this month.\n";
        return ss.str();
    }

    _display_project_breakdown(ss, data, db);
    return ss.str();
}

void MonthlyReportMarkdownFormatter::_display_summary(std::stringstream& ss, const MonthlyReportData& data) const {
    ss << "## Monthly Summary for " << data.year_month.substr(0, 4) << "-" << data.year_month.substr(4, 2) << "\n\n";

    if (data.actual_days > 0) {
        ss << "- **Actual Days with Records**: " << data.actual_days << "\n";
        ss << "- **Total Time Recorded**: " << time_format_duration(data.total_duration, data.actual_days) << "\n";
    }
}

void MonthlyReportMarkdownFormatter::_display_project_breakdown(std::stringstream& ss, const MonthlyReportData& data, sqlite3* db) const {
    ProjectBreakdownFormatter formatter(db, data.records, data.total_duration, data.actual_days);
    formatter.write_to_stream(ss);
}