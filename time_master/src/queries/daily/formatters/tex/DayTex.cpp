#include "DayTex.hpp"
#include "DayTexConfig.hpp" // [MODIFIED] Include the simplified configuration file

#include <iomanip>
#include <string>
#include <sstream>

#include "common/common_utils.hpp"
#include "queries/shared/utils/query_utils.hpp"
#include "queries/shared/utils/BoolToString.hpp"
#include "queries/shared/factories/TreeFmtFactory.hpp"
#include "queries/shared/Interface/ITreeFmt.hpp"
#include "queries/shared/data/DailyReportData.hpp"

// Local helper function to escape special TeX characters.
namespace {
    std::string escape_tex_local(const std::string& s) {
        std::string escaped;
        escaped.reserve(s.length());
        for (char c : s) {
            if (c == '&' || c == '%' || c == '$' || c == '#' || c == '_' || c == '{' || c == '}') {
                escaped += '\\';
            }
            escaped += c;
        }
        return escaped;
    }
}

std::string DayTex::format_report(const DailyReportData& data, sqlite3* db) const {
    return format_report_template(data, db);
}

void DayTex::format_content(std::stringstream& ss, const DailyReportData& data, sqlite3* db) const {
    _display_header(ss, data);

    if (data.total_duration == 0) {
        // [MODIFIED] Use the plain text message and add the newline here.
        ss << DayTexConfig::NoRecordsMessage << "\n";
    } else {
        _display_project_breakdown(ss, data, db);
    }
}

void DayTex::_display_header(std::stringstream& ss, const DailyReportData& data) const {
    // [MODIFIED] TeX syntax is now hardcoded in this function. It dynamically
    // inserts the display text from the DayTexConfig namespace.
    
    // Title: Combines the configurable title with the date variable.
    ss << "\\section*{" << DayTexConfig::ReportTitle << " " << escape_tex_local(data.date) << "}\n\n";
    
    // Metadata list: Wraps configurable labels within TeX item/bold commands.
    ss << "\\begin{itemize}\n";
    
    ss << "    \\item \\textbf{" << DayTexConfig::DateLabel      << "}: " << escape_tex_local(data.date) << "\n";
    ss << "    \\item \\textbf{" << DayTexConfig::TotalTimeLabel << "}: " << escape_tex_local(time_format_duration(data.total_duration)) << "\n";
    ss << "    \\item \\textbf{" << DayTexConfig::StatusLabel    << "}: " << escape_tex_local(bool_to_string(data.metadata.status)) << "\n";
    ss << "    \\item \\textbf{" << DayTexConfig::SleepLabel     << "}: " << escape_tex_local(bool_to_string(data.metadata.sleep)) << "\n";
    ss << "    \\item \\textbf{" << DayTexConfig::GetupTimeLabel << "}: " << escape_tex_local(data.metadata.getup_time) << "\n";
    ss << "    \\item \\textbf{" << DayTexConfig::RemarkLabel    << "}: " << escape_tex_local(data.metadata.remark) << "\n";

    ss << "\\end{itemize}\n\n";
}

void DayTex::_display_project_breakdown(std::stringstream& ss, const DailyReportData& data, sqlite3* db) const {
    ss << generate_project_breakdown(
        ReportFormat::LaTeX, 
        db, 
        data.records, 
        data.total_duration, 
        1 
    );
}