// queries/monthly/formatters/tex/MonthTex.cpp
#include "MonthTex.hpp"
#include "MonthTexConfig.hpp" // [MODIFIED] Include the new configuration file

#include <iomanip>
#include <string>
#include <sstream>

#include "queries/shared/utils/query_utils.hpp"
#include "queries/shared/factories/TreeFmtFactory.hpp"
#include "queries/shared/Interface/ITreeFmt.hpp"
#include "common/utils/ProjectTree.hpp" // For ProjectNode, ProjectTree"

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

std::string MonthTex::format_report(const MonthlyReportData& data, sqlite3* db) const {
    if (data.year_month == "INVALID") {
        // [MODIFIED] Use the configurable error message.
        return std::string(MonthTexConfig::InvalidFormatMessage) + "\n";
    }
    return format_report_template(data, db);
}

void MonthTex::format_content(std::stringstream& ss, const MonthlyReportData& data, sqlite3* db) const {
    _display_summary(ss, data);

    if (data.actual_days == 0) {
        // [MODIFIED] Use the configurable message and add a newline.
        ss << MonthTexConfig::NoRecordsMessage << "\n";
    } else {
        _display_project_breakdown(ss, data, db);
    }
}

void MonthTex::_display_summary(std::stringstream& ss, const MonthlyReportData& data) const {
    // [MODIFIED] TeX syntax is now hardcoded here and combined with labels from the config file.
    
    std::string title_month = data.year_month.substr(0, 4) + "-" + data.year_month.substr(4, 2);
    ss << "\\section*{" << MonthTexConfig::ReportTitle << " " << escape_tex_local(title_month) << "}\n\n";

    if (data.actual_days > 0) {
        ss << "\\begin{itemize}\n";
        ss << "    \\item \\textbf{" << MonthTexConfig::ActualDaysLabel << "}: " << data.actual_days << "\n";
        ss << "    \\item \\textbf{" << MonthTexConfig::TotalTimeLabel  << "}: " << escape_tex_local(time_format_duration(data.total_duration, data.actual_days)) << "\n";
        ss << "\\end{itemize}\n\n";
    }
}

void MonthTex::_display_project_breakdown(std::stringstream& ss, const MonthlyReportData& data, sqlite3* db) const {
    // This function remains unchanged as it calls a separate utility.
    ss << generate_project_breakdown(
        ReportFormat::LaTeX,
        db,
        data.records,
        data.total_duration,
        data.actual_days
    );
}