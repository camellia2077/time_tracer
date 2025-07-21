// PeriodTex.cpp
#include "PeriodTex.h"
#include <iomanip>
#include <string>

// --- 核心改动：包含新的依赖 ---
#include "report_generators/_shared/query_utils.h"      // 用于 build_project_tree_from_records 和 get_parent_map
#include "format/ProjectBreakdownFormatterFactory.h" // 新的工厂
#include "format/IProjectBreakdownFormatter.h"     // 工厂返回的接口
#include "common_utils.h"                                // 为了 ProjectTree

// --- _escape_tex 函数已被移除 ---

// format_report 方法的逻辑保持不变
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
        // 调用更新后的 _display_project_breakdown
        _display_project_breakdown(ss, data, db);
    }

    ss << "\n\\end{document}\n";
    return ss.str();
}

// _display_preamble 方法保持不变
void PeriodTex::_display_preamble(std::stringstream& ss) const {
    ss << "\\documentclass{article}\n";
    ss << "\\usepackage[a4paper, margin=1in]{geometry}\n";
    ss << "\\usepackage{fontspec}\n";
    ss << "\\usepackage{ctex}\n";
    ss << "\\setmainfont{Noto Serif SC}\n";
    ss << "\\setCJKmainfont{Noto Serif SC}\n\n";
    ss << "\\begin{document}\n\n";
}

// _display_summary 方法保持不变，但内部需要一个临时的转义逻辑
void PeriodTex::_display_summary(std::stringstream& ss, const PeriodReportData& data) const {
    auto escape_tex_local = [](const std::string& s) {
        std::string escaped;
        for (char c : s) {
            if (c == '&' || c == '%' || c == '$' || c == '#' || c == '_' || c == '{' || c == '}') {
                escaped += '\\';
            }
            escaped += c;
        }
        return escaped;
    };

    ss << "\\section*{Period Report: Last " << data.days_to_query << " days ("
       << escape_tex_local(data.start_date) << " to " << escape_tex_local(data.end_date) << ")}\n\n";

    if (data.actual_days > 0) {
        ss << "\\begin{itemize}\n";
        ss << "    \\item \\textbf{Total Time Recorded}: " << escape_tex_local(time_format_duration(data.total_duration, data.actual_days)) << "\n";
        ss << "    \\item \\textbf{Actual Days with Records}: " << data.actual_days << "\n";
        ss << "\\end{itemize}\n\n";
    }
}


// --- 核心改动：_display_project_breakdown 的全新实现 ---
void PeriodTex::_display_project_breakdown(std::stringstream& ss, const PeriodReportData& data, sqlite3* db) const {
    // 1. 准备数据：构建项目树
    std::map<std::string, std::string> parent_map = get_parent_map(db);
    ProjectTree project_tree;
    build_project_tree_from_records(project_tree, data.records, parent_map);

    // 2. 使用工厂创建 LaTeX 格式化器
    auto formatter = ProjectBreakdownFormatterFactory::createFormatter(ReportFormat::LaTex);

    // 3. 调用格式化器并获取结果
    if (formatter) {
        // 调用新接口的 format 方法
        std::string breakdown_output = formatter->format(project_tree, data.total_duration, data.actual_days);
        ss << breakdown_output;
    }
}