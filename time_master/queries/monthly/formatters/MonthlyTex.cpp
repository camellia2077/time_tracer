// MonthlyTex.cpp (已更新)
#include "MonthlyTex.h"
#include <iomanip>
#include <string>

// --- 依赖保持不变 ---
#include "queries/shared/query_utils.h"
#include "queries/shared/breakdown/ProjectBreakdownFormatterFactory.h"
#include "queries/shared/breakdown/IProjectBreakdownFormatter.h"
#include "common/common_utils.h"

// 1. 公开的接口现在只负责调用基类的模板方法
std::string MonthlyTex::format_report(const MonthlyReportData& data, sqlite3* db) const {
    if (data.year_month == "INVALID") {
        // 对于无效输入，直接返回错误信息，不使用模板
        return "Invalid year_month format. Expected YYYYMM.\n";
    }
    return format_report_template(data, db);
}

// 2. 实现新的 format_content 方法，将所有核心内容生成逻辑移到这里
void MonthlyTex::format_content(std::stringstream& ss, const MonthlyReportData& data, sqlite3* db) const {
    // 步骤 A: 生成报告总结
    _display_summary(ss, data);

    // 步骤 B: 生成报告主体
    if (data.actual_days == 0) {
        ss << "No time records found for this month.\n";
    } else {
        _display_project_breakdown(ss, data, db);
    }
}

// 3. _display_preamble 方法被彻底删除

// _display_summary 方法保持不变
void MonthlyTex::_display_summary(std::stringstream& ss, const MonthlyReportData& data) const {
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

    std::string title_month = data.year_month.substr(0, 4) + "-" + data.year_month.substr(4, 2);
    ss << "\\section*{Monthly Summary for " << escape_tex_local(title_month) << "}\n\n";

    if (data.actual_days > 0) {
        ss << "\\begin{itemize}\n";
        ss << "    \\item \\textbf{Actual Days with Records}: " << data.actual_days << "\n";
        ss << "    \\item \\textbf{Total Time Recorded}: " << escape_tex_local(time_format_duration(data.total_duration, data.actual_days)) << "\n";
        ss << "\\end{itemize}\n\n";
    }
}

// _display_project_breakdown 方法保持不变
void MonthlyTex::_display_project_breakdown(std::stringstream& ss, const MonthlyReportData& data, sqlite3* db) const {
    std::map<std::string, std::string> parent_map = get_parent_map(db);
    ProjectTree project_tree;
    build_project_tree_from_records(project_tree, data.records, parent_map);

    auto formatter = ProjectBreakdownFormatterFactory::createFormatter(ReportFormat::LaTeX);
    if (formatter) {
        std::string breakdown_output = formatter->format(project_tree, data.total_duration, data.actual_days);
        ss << breakdown_output;
    }
}