#include "PeriodTex.h"
#include "report_generators/_shared/query_utils.h"
#include <iomanip>
#include <string>
#include <map>
#include <algorithm>

// --- Utility function to escape TeX special characters (reused from MonthlyTex) ---
std::string PeriodTex::_escape_tex(const std::string& s) const {
    std::string escaped;
    escaped.reserve(s.length());
    for (char c : s) {
        switch (c) {
            case '&':  escaped += "\\&";        break;
            case '%':  escaped += "\\%";        break;
            case '$':  escaped += "\\$";        break;
            case '#':  escaped += "\\#";        break;
            case '_':  escaped += "\\_";        break;
            case '{':  escaped += "\\{";        break;
            case '}':  escaped += "\\}";        break;
            case '~':  escaped += "\\textasciitilde{}"; break;
            case '^':  escaped += "\\textasciicircum{}"; break;
            case '\\': escaped += "\\textbackslash{}"; break;
            default:   escaped += c;            break;
        }
    }
    return escaped;
}

// --- PeriodTex Class Implementation ---

std::string PeriodTex::format_report(const PeriodReportData& data, sqlite3* db) const {
    std::stringstream ss;
    if (data.days_to_query <= 0) {
        return "Number of days to query must be positive.\n";
    }

    _display_preamble(ss);
    _display_summary(ss, data);

    if (data.actual_days == 0) {
        ss << "No time records found in this period.\n";
    } else {
        _display_project_breakdown(ss, data, db);
    }

    ss << "\n\\end{document}\n";
    return ss.str();
}

// Reused directly from MonthlyTex.cpp
void PeriodTex::_display_preamble(std::stringstream& ss) const {
    ss << "\\documentclass{article}\n";
    ss << "\\usepackage[a4paper, margin=1in]{geometry}\n";
    ss << "\\usepackage{longtable}\n";
    ss << "\\usepackage{graphicx}\n";
    ss << "\\usepackage{hyperref}\n";
    ss << "\\usepackage{fontspec}\n";
    ss << "\\usepackage{ctex}\n";
    ss << "\\setmainfont{Noto Serif SC}\n";
    ss << "\\setCJKmainfont{Noto Serif SC}\n\n";
    ss << "\\begin{document}\n\n";
}

// Period specific summary
void PeriodTex::_display_summary(std::stringstream& ss, const PeriodReportData& data) const {
    ss << "\\section*{Period Report: Last " << data.days_to_query << " days ("
       << _escape_tex(data.start_date) << " to " << _escape_tex(data.end_date) << ")}\n\n";

    if (data.actual_days > 0) {
        ss << "\\begin{itemize}\n";
        ss << "    \\item \\textbf{Total Time Recorded}: " << _escape_tex(time_format_duration(data.total_duration, data.actual_days)) << "\n";
        ss << "    \\item \\textbf{Actual Days with Records}: " << data.actual_days << "\n";
        ss << "\\end{itemize}\n\n";
    }
}

// Period specific project breakdown (reusing ProjectBreakdownFormatter if applicable, or adapt similar logic to MonthlyTex)
void PeriodTex::_display_project_breakdown(std::stringstream& ss, const PeriodReportData& data, sqlite3* db) const {
    ss << "\\subsection*{Project Breakdown}\n\n";

    // Reusing the ProjectBreakdownFormatter logic, adapted for TeX output.
    // This assumes ProjectBreakdownFormatter can be modified to accept a stream
    // and format its output as TeX, or that its internal logic is exposed
    // for this class to format directly.
    // For simplicity, I'm mimicking the table structure from MonthlyTex.cpp
    // and applying the same calculation logic.

    std::map<std::string, double> project_durations;
    for (const auto& record : data.records) {
        project_durations[record.first] += record.second;
    }

    ss << "\\begin{longtable}{| l | l | l | r |}\n";
    ss << "\\hline\n";
    ss << "\\textbf{Project} & \\textbf{Total Time} & \\textbf{Avg Daily Time} & \\textbf{Percentage} \\\\\n";
    ss << "\\hline\n";
    ss << "\\endfirsthead\n";
    ss << "\\hline\n";
    ss << "\\textbf{Project} & \\textbf{Total Time} & \\textbf{Avg Daily Time} & \\textbf{Percentage} \\\\\n";
    ss << "\\hline\n";
    ss << "\\endhead\n";
    ss << "\\hline\n";
    ss << "\\endfoot\n";
    ss << "\\hline\n";
    ss << "\\endlastfoot\n";

    for (const auto& pair : project_durations) {
        double total_project_duration = pair.second;
        double avg_daily_duration = (data.actual_days > 0) ? (total_project_duration / data.actual_days) : 0;
        double percentage = (data.total_duration > 0) ? (total_project_duration / data.total_duration * 100.0) : 0;

        ss << _escape_tex(pair.first) << " & "
           << _escape_tex(time_format_duration(total_project_duration)) << " & "
           << _escape_tex(time_format_duration(avg_daily_duration)) << " & "
           << std::fixed << std::setprecision(2) << percentage << "\\% \\\\\n";
        ss << "\\hline\n";
    }

    ss << "\\end{longtable}\n";
}