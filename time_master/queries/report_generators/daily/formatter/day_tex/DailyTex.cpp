#include "DailyTex.h"
#include "report_generators/_shared/query_utils.h" // 假设 ProjectBreakdownFormatter 在这里
#include <iomanip>
#include <string>
#include <map>
#include <algorithm>

// --- Utility function to escape TeX special characters ---
std::string DailyTex::_escape_tex(const std::string& s) const {
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


// --- DailyTex Class Implementation ---

std::string DailyTex::format_report(const DailyReportData& data, sqlite3* db) {
    std::stringstream ss;
    
    _display_preamble(ss);
    _display_header(ss, data);

    if (data.total_duration == 0) {
        ss << "No time records for this day.\n";
    } else {
        _display_project_breakdown(ss, data, db);
    }

    ss << "\n\\end{document}\n";
    return ss.str();
}

void DailyTex::_display_preamble(std::stringstream& ss) const {
    ss << "\\documentclass{article}\n";
    ss << "\\usepackage[a4paper, margin=1in]{geometry}\n";
    ss << "\\usepackage{longtable}\n";
    ss << "\\usepackage{graphicx}\n";
    ss << "\\usepackage{hyperref}\n";

    // --- Font Configuration for XeLaTeX ---
    ss << "\\usepackage{fontspec}\n";
    ss << "\\usepackage{ctex}\n";
    // 将主字体（用于英文、数字）和CJK字体（用于中文）都设置为 Noto Serif SC
    ss << "\\setmainfont{Noto Serif SC}\n";
    ss << "\\setCJKmainfont{Noto Serif SC}\n\n";

    ss << "\\begin{document}\n\n";
}

void DailyTex::_display_header(std::stringstream& ss, const DailyReportData& data) const {
    ss << "\\section*{Daily Report for " << _escape_tex(data.date) << "}\n\n";
    ss << "\\begin{itemize}\n";
    ss << "    \\item \\textbf{Date}: " << _escape_tex(data.date) << "\n";
    ss << "    \\item \\textbf{Total Time Recorded}: " << _escape_tex(time_format_duration(data.total_duration)) << "\n";
    ss << "    \\item \\textbf{Status}: " << _escape_tex(data.metadata.status) << "\n";
    ss << "    \\item \\textbf{Getup Time}: " << _escape_tex(data.metadata.getup_time) << "\n";
    ss << "    \\item \\textbf{Remark}: " << _escape_tex(data.metadata.remark) << "\n";
    ss << "\\end{itemize}\n\n";
}

void DailyTex::_display_project_breakdown(std::stringstream& ss, const DailyReportData& data, sqlite3* db) const {
    ss << "\\subsection*{Project Breakdown}\n\n";
    
    // 与 Markdown 版本类似，我们假设有一个能够处理 TeX 格式的 ProjectBreakdownFormatter
    // 或者直接在此处实现其逻辑。为了完整性，这里将直接实现表格生成逻辑。
    
    // 假设 data.records 是一个包含所有记录的向量，我们需要按项目分组和汇总
    std::map<std::string, double> project_durations;
    for (const auto& record : data.records) {
        project_durations[record.first] += record.second;
    }

    ss << "\\begin{longtable}{| l | l | r |}\n";
    ss << "\\hline\n";
    ss << "\\textbf{Project} & \\textbf{Time Spent} & \\textbf{Percentage} \\\\\n";
    ss << "\\hline\n";
    ss << "\\endfirsthead\n";
    ss << "\\hline\n";
    ss << "\\textbf{Project} & \\textbf{Time Spent} & \\textbf{Percentage} \\\\\n";
    ss << "\\hline\n";
    ss << "\\endhead\n";
    ss << "\\hline\n";
    ss << "\\endfoot\n";
    ss << "\\hline\n";
    ss << "\\endlastfoot\n";

    for (const auto& pair : project_durations) {
        double percentage = (data.total_duration > 0) ? (pair.second / data.total_duration * 100.0) : 0;
        ss << _escape_tex(pair.first) << " & "
           << _escape_tex(time_format_duration(pair.second)) << " & "
           << std::fixed << std::setprecision(2) << percentage << "\\% \\\\\n";
        ss << "\\hline\n";
    }

    ss << "\\end{longtable}\n";
}