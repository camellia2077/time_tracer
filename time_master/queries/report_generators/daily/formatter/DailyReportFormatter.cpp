#include "DailyReportFormatter.h"
#include "report_generators/_shared/query_utils.h" // For time_format_duration and now ProjectBreakdownFormatter
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
// 输出标题行
void DailyReportFormatter::_display_header(std::stringstream& ss, const DailyReportData& data) {
    // Use a level 2 heading for the main title
    ss << "## Daily Report for " << data.date << "\n\n"; 

    // Use a bulleted list for the metadata items
    ss << "- **Date**: " << data.date << "\n";
    ss << "- **Total Time Recorded**: " << time_format_duration(data.total_duration) << "\n";
    ss << "- **Status**: " << data.metadata.status << "\n";
    ss << "- **Getup Time**: " << data.metadata.getup_time << "\n";
    
    // Use a blockquote for the remark to make it stand out
    ss << "- **Remark**:" << data.metadata.remark << "\n";
}
// 输出内容行
void DailyReportFormatter::_display_project_breakdown(std::stringstream& ss, const DailyReportData& data, sqlite3* db) {
    // This function is now decoupled from the global write_project_breakdown_to_stream
    // and uses the new formatter class, improving modularity.
    ProjectBreakdownFormatter formatter(db, data.records, data.total_duration, 1);
    formatter.write_to_stream(ss);
}