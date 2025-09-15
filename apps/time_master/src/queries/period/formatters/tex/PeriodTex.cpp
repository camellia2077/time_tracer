// queries/period/formatters/tex/PeriodTex.cpp
#include "PeriodTex.hpp"
#include "PeriodTexConfig.hpp"

#include <iomanip>
#include <string>
#include <sstream>

#include "queries/shared/utils/query_utils.hpp"
#include "queries/shared/factories/TreeFmtFactory.hpp"
#include "common/utils/ProjectTree.hpp"

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

std::string PeriodTex::format_report(const PeriodReportData& data, sqlite3* db) const {
    if (data.days_to_query <= 0) {
        return std::string(PeriodTexConfig::InvalidDaysMessage) + "\n";
    }
    return format_report_template(data, db);
}

void PeriodTex::format_content(std::stringstream& ss, const PeriodReportData& data, sqlite3* db) const {
    _display_summary(ss, data);
    if (data.actual_days == 0) {
        ss << PeriodTexConfig::NoRecordsMessage << "\n";
    } else {
        _display_project_breakdown(ss, data, db);
    }
}

void PeriodTex::_display_summary(std::stringstream& ss, const PeriodReportData& data) const {
    ss << "\\section*{"
       << PeriodTexConfig::ReportTitlePrefix << " " << data.days_to_query << " "
       << PeriodTexConfig::ReportTitleDays << " ("
       << escape_tex_local(data.start_date) << " " << PeriodTexConfig::ReportTitleDateSeparator << " "
       << escape_tex_local(data.end_date) << ")}\n\n";

    if (data.actual_days > 0) {
        // [核心修改]
        ss << "\\begin{itemize}" << PeriodTexConfig::CompactListOptions << "\n";
        ss << "    \\item \\textbf{" << PeriodTexConfig::TotalTimeLabel << "}: " 
           << escape_tex_local(time_format_duration(data.total_duration, data.actual_days)) << "\n";
        ss << "    \\item \\textbf{" << PeriodTexConfig::ActualDaysLabel << "}: " 
           << data.actual_days << "\n";
        ss << "\\end{itemize}\n\n";
    }
}

void PeriodTex::_display_project_breakdown(std::stringstream& ss, const PeriodReportData& data, sqlite3* db) const {
    ss << generate_project_breakdown(
        ReportFormat::LaTeX,
        db,
        data.records,
        data.total_duration,
        data.actual_days
    );
}