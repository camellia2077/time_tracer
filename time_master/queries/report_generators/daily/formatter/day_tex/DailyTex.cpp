// DailyTex.cpp
#include "DailyTex.h"
#include <iomanip>
#include <string>

// --- 核心改动：包含新的依赖 ---
#include "report_generators/_shared/query_utils.h"      // 用于 build_project_tree_from_records 和 get_parent_map
#include "format/ProjectBreakdownFormatterFactory.h" // 新的工厂
#include "format/IProjectBreakdownFormatter.h"     // 工厂返回的接口
#include "common_utils.h"                                // 为了 ProjectTree

// --- _escape_tex 函数已被移除，不再需要 ---

// format_report 方法保持不变
std::string DailyTex::format_report(const DailyReportData& data, sqlite3* db) {
    std::stringstream ss;
    
    _display_preamble(ss);
    _display_header(ss, data);

    if (data.total_duration == 0) {
        ss << "No time records for this day.\n";
    } else {
        // 调用更新后的 _display_project_breakdown
        _display_project_breakdown(ss, data, db);
    }

    ss << "\n\\end{document}\n";
    return ss.str();
}

// _display_preamble 和 _display_header 方法保持不变
void DailyTex::_display_preamble(std::stringstream& ss) const {
    ss << "\\documentclass{article}\n";
    ss << "\\usepackage[a4paper, margin=1in]{geometry}\n";
    ss << "\\usepackage{fontspec}\n";
    ss << "\\usepackage{ctex}\n";
    ss << "\\setmainfont{Noto Serif SC}\n";
    ss << "\\setCJKmainfont{Noto Serif SC}\n\n";
    ss << "\\begin{document}\n\n";
}

void DailyTex::_display_header(std::stringstream& ss, const DailyReportData& data) const {
    // 为确保安全，我们在这里仍然需要一个临时的转义函数，或者将 DailyTex::_escape_tex 保留
    // 为了简单起见，我们在这里直接实现一个简单的转义
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

    ss << "\\section*{Daily Report for " << escape_tex_local(data.date) << "}\n\n";
    ss << "\\begin{itemize}\n";
    ss << "    \\item \\textbf{Date}: " << escape_tex_local(data.date) << "\n";
    ss << "    \\item \\textbf{Total Time Recorded}: " << escape_tex_local(time_format_duration(data.total_duration)) << "\n";
    ss << "    \\item \\textbf{Status}: " << escape_tex_local(data.metadata.status) << "\n";
    ss << "    \\item \\textbf{Getup Time}: " << escape_tex_local(data.metadata.getup_time) << "\n";
    ss << "    \\item \\textbf{Remark}: " << escape_tex_local(data.metadata.remark) << "\n";
    ss << "\\end{itemize}\n\n";
}


// --- 核心改动：_display_project_breakdown 的全新实现 ---
void DailyTex::_display_project_breakdown(std::stringstream& ss, const DailyReportData& data, sqlite3* db) const {
    // 1. 准备数据：构建项目树
    std::map<std::string, std::string> parent_map = get_parent_map(db);
    ProjectTree project_tree;
    build_project_tree_from_records(project_tree, data.records, parent_map);

    // 2. 使用工厂创建 LaTeX 格式化器
    auto formatter = ProjectBreakdownFormatterFactory::createFormatter(ReportFormat::LaTeX);

    // 3. 调用格式化器并获取结果
    if (formatter) {
        // 调用新接口的 format 方法
        std::string breakdown_output = formatter->format(project_tree, data.total_duration, 1);
        ss << breakdown_output;
    }
}