// PeriodTex.cpp (已更新)
#include "PeriodTex.h"
#include <iomanip>
#include <string>

// --- 依赖保持不变 ---
#include "queries/_shared/query_utils.h"
#include "queries/formatters/ProjectBreakdownFormatterFactory.h"
#include "queries/formatters/IPeriodReportFormatter.h"
#include "common/common_utils.h"

// 1. 公开的接口现在只负责调用基类的模板方法
std::string PeriodTex::format_report(const PeriodReportData& data, sqlite3* db) const {
    if (data.days_to_query <= 0) {
        // 对于无效输入，直接返回错误信息，不使用模板
        return "Number of days to query must be positive.\n";
    }
    return format_report_template(data, db);
}

// 2. 实现新的 format_content 方法，将所有核心内容生成逻辑移到这里
void PeriodTex::format_content(std::stringstream& ss, const PeriodReportData& data, sqlite3* db) const {
    // 步骤 A: 生成报告总结
    _display_summary(ss, data);

    // 步骤 B: 生成报告主体
    if (data.actual_days == 0) {
        ss << "No time records found in this period.\n";
    } else {
        _display_project_breakdown(ss, data, db);
    }
}

// 3. _display_preamble 方法被彻底删除

// _display_summary 方法保持不变
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

// _display_project_breakdown 方法保持不变
void PeriodTex::_display_project_breakdown(std::stringstream& ss, const PeriodReportData& data, sqlite3* db) const {
    std::map<std::string, std::string> parent_map = get_parent_map(db);
    ProjectTree project_tree;
    build_project_tree_from_records(project_tree, data.records, parent_map);

    auto formatter = ProjectBreakdownFormatterFactory::createFormatter(ReportFormat::LaTeX);
    if (formatter) {
        std::string breakdown_output = formatter->format(project_tree, data.total_duration, data.actual_days);
        ss << breakdown_output;
    }
}