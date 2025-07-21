#include "MonthlyTex.h"
#include "report_generators/_shared/query_utils.h"
#include <iomanip>
#include <string>
#include <map>
#include <algorithm>

// --- Utility function to escape TeX special characters (reused from DailyTex) ---
std::string MonthlyTex::_escape_tex(const std::string& s) const {
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

// --- MonthlyTex Class Implementation ---

std::string MonthlyTex::format_report(const MonthlyReportData& data, sqlite3* db) const {
    std::stringstream ss;
    if (data.year_month == "INVALID") {
        // 对于无效数据，直接返回错误信息，不生成 TeX 文档结构
        return "Invalid year_month format. Expected YYYYMM.\n";
    }

    _display_preamble(ss);
    _display_summary(ss, data);

    if (data.actual_days == 0) {
        ss << "No time records found for this month.\n";
    } else {
        _display_project_breakdown(ss, data, db);
    }

    ss << "\n\\end{document}\n";
    return ss.str();
}

// Reused directly from DailyTex.cpp
void MonthlyTex::_display_preamble(std::stringstream& ss) const {
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

// Monthly specific summary
void MonthlyTex::_display_summary(std::stringstream& ss, const MonthlyReportData& data) const {
    std::string title_month = data.year_month.substr(0, 4) + "-" + data.year_month.substr(4, 2);
    ss << "\\section*{Monthly Summary for " << _escape_tex(title_month) << "}\n\n";

    if (data.actual_days > 0) {
        ss << "\\begin{itemize}\n";
        ss << "    \\item \\textbf{Actual Days with Records}: " << data.actual_days << "\n";
        ss << "    \\item \\textbf{Total Time Recorded}: " << _escape_tex(time_format_duration(data.total_duration, data.actual_days)) << "\n";
        ss << "\\end{itemize}\n\n";
    }
}

// Monthly specific project breakdown
void MonthlyTex::_display_project_breakdown(std::stringstream& ss, const MonthlyReportData& data, sqlite3* db) const {
    ss << "\\subsection*{Project Breakdown}\n\n";
    
    std::map<std::string, double> project_durations;
    for (const auto& record : data.records) {
        project_durations[record.first] += record.second;
    }

    // A 4-column table for the monthly breakdown
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