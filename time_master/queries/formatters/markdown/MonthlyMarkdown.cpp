// queries/report_generators/monthly/formatter/month_md/MonthlyMarkdown.cpp
#include "MonthlyMarkdown.h"
#include <iomanip>

#include "queries/_shared/query_utils.h"      // 用于 build_project_tree_from_records 和 get_parent_map
#include "queries/formatters/ProjectBreakdownFormatterFactory.h" // 新的工厂
#include "queries/formatters/IProjectBreakdownFormatter.h"     // 工厂返回的接口
#include "common/common_utils.h"                                // 为了 ProjectTree

// format_report 方法的逻辑保持不变，它将工作委托给辅助函数
std::string MonthlyMarkdown::format_report(const MonthlyReportData& data, sqlite3* db) const {
    std::stringstream ss;
    if (data.year_month == "INVALID") {
        ss << "Invalid year_month format. Expected YYYYMM.\n";
        return ss.str();
    }

    _display_summary(ss, data);

    if (data.actual_days == 0) {
        ss << "No time records found for this month.\n";
        return ss.str();
    }
    
    // 调用更新后的 _display_project_breakdown
    _display_project_breakdown(ss, data, db);
    return ss.str();
}

// _display_summary 方法保持不变
void MonthlyMarkdown::_display_summary(std::stringstream& ss, const MonthlyReportData& data) const {
    ss << "## Monthly Summary for " << data.year_month.substr(0, 4) << "-" << data.year_month.substr(4, 2) << "\n\n";

    if (data.actual_days > 0) {
        ss << "- **Actual Days with Records**: " << data.actual_days << "\n";
        ss << "- **Total Time Recorded**: " << time_format_duration(data.total_duration, data.actual_days) << "\n";
    }
}

// --- 核心改动：_display_project_breakdown 的全新实现 ---
void MonthlyMarkdown::_display_project_breakdown(std::stringstream& ss, const MonthlyReportData& data, sqlite3* db) const {
    // 1. 准备数据：构建项目树
    // 这些是来自 query_utils.cpp 的独立工具函数
    std::map<std::string, std::string> parent_map = get_parent_map(db);
    ProjectTree project_tree;
    build_project_tree_from_records(project_tree, data.records, parent_map);

    // 2. 使用工厂创建格式化器
    auto formatter = ProjectBreakdownFormatterFactory::createFormatter(ReportFormat::Markdown);

    // 3. 调用格式化器并获取结果
    if (formatter) {
        // 调用新接口的 format 方法
        std::string breakdown_output = formatter->format(project_tree, data.total_duration, data.actual_days);
        ss << breakdown_output;
    }
}