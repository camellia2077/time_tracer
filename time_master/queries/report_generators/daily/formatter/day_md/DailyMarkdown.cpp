#include "DailyMarkdown.h"
#include "report_generators/_shared/query_utils.h"
#include <iomanip>

// --- DailyMarkdown Class Implementation ---

// The 'const' keyword has been removed from the function definition to match the header
std::string DailyMarkdown::format_report(const DailyReportData& data, sqlite3* db) {
    std::stringstream ss;
    _display_header(ss, data);

    if (data.total_duration == 0) {
        ss << "No time records for this day.\n";
        return ss.str();
    }
    
    _display_project_breakdown(ss, data, db);
    return ss.str();
}

void DailyMarkdown::_display_header(std::stringstream& ss, const DailyReportData& data) const {
    ss << "## Daily Report for " << data.date << "\n\n"; 
    ss << "- **Date**: " << data.date << "\n";
    ss << "- **Total Time Recorded**: " << time_format_duration(data.total_duration) << "\n";
    ss << "- **Status**: " << data.metadata.status << "\n";
    ss << "- **Getup Time**: " << data.metadata.getup_time << "\n";
    ss << "- **Remark**:" << data.metadata.remark << "\n";
}

void DailyMarkdown::_display_project_breakdown(std::stringstream& ss, const DailyReportData& data, sqlite3* db) const {
    ProjectBreakdownFormatter formatter(db, data.records, data.total_duration, 1);
    formatter.write_to_stream(ss);
}