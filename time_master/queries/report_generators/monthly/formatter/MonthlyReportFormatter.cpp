#include "MonthlyReportFormatter.h"
#include "query_utils.h" // 假设此文件在 report_generators 目录下
#include <iomanip>

std::string MonthlyReportFormatter::format_report(const MonthlyReportData& data, sqlite3* db) {
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

void MonthlyReportFormatter::_display_summary(std::stringstream& ss, const MonthlyReportData& data) {
    ss << "\n--- Monthly Summary for " << data.year_month.substr(0, 4) << "-" << data.year_month.substr(4, 2) << " ---\n";
    if (data.actual_days > 0) {
        ss << "Actual Days with Records: " << data.actual_days << "\n";
        ss << "Total Time Recorded: " << time_format_duration(data.total_duration, data.actual_days) << "\n";
    }
    ss << "-------------------------------------\n";
}

void MonthlyReportFormatter::_display_project_breakdown(std::stringstream& ss, const MonthlyReportData& data, sqlite3* db) {
    write_project_breakdown_to_stream(ss, db, data.records, data.total_duration, data.actual_days);
}