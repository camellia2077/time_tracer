#include "DailyReportFormatter.h"
#include "query_utils.h" // For time_format_duration and write_project_breakdown_to_stream
#include <iomanip>

// --- DailyReportFormatter Class Implementation ---

std::string DailyReportFormatter::format_report(const DailyReportData& data, sqlite3* db) {
    std::stringstream ss;
    _display_header(ss, data);

    if (data.total_duration == 0) {
        ss << "No time records for this day.\n";
        return ss.str();
    }
    
    _display_project_breakdown(ss, data, db);
    return ss.str();
}

void DailyReportFormatter::_display_header(std::stringstream& ss, const DailyReportData& data) {
    ss << "\n--- Daily Report for " << data.date << " ---\n";
    ss << "Date: " << data.date << "\n";
    ss << "Total Time Recorded: " << time_format_duration(data.total_duration) << "\n";
    ss << "Status: " << data.metadata.status << "\n";
    ss << "Getup Time: " << data.metadata.getup_time << "\n";
    ss << "Remark: " << data.metadata.remark << "\n";
    ss << "-------------------------------------\n";
}

void DailyReportFormatter::_display_project_breakdown(std::stringstream& ss, const DailyReportData& data, sqlite3* db) {
    // This function remains coupled to query_utils, which is acceptable
    // as it is part of the formatting logic.
    write_project_breakdown_to_stream(ss, db, data.records, data.total_duration, 1);
}