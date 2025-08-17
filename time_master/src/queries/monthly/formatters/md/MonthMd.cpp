
// queries/report_generators/monthly/formatter/month_md/MonthMd.cpp
#include "MonthMd.h"
#include <iomanip>

#include "queries/shared/query_utils.h"      // 用于 build_project_tree_from_records 和 get_parent_map
#include "queries/shared/breakdown/TreeFmtFactory.h" // 新的工厂
#include "queries/shared/breakdown/ITreeFmt.h"     // 工厂返回的接口
#include "common/common_utils.h"                                // 为了 ProjectTree

// format_report 方法的逻辑保持不变，它将工作委托给辅助函数
std::string MonthMd::format_report(const MonthlyReportData& data, sqlite3* db) const {
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
void MonthMd::_display_summary(std::stringstream& ss, const MonthlyReportData& data) const {
    ss << "## Monthly Summary for " << data.year_month.substr(0, 4) << "-" << data.year_month.substr(4, 2) << "\n\n";

    if (data.actual_days > 0) {
        ss << "- **Actual Days with Records**: " << data.actual_days << "\n";
        ss << "- **Total Time Recorded**: " << time_format_duration(data.total_duration, data.actual_days) << "\n";
    }
}

void MonthMd::_display_project_breakdown(std::stringstream& ss, const MonthlyReportData& data, sqlite3* db) const {
    // 调用统一的工具函数来生成项目明细的 Markdown 格式字符串
    ss << generate_project_breakdown(
        ReportFormat::Markdown,     // 指定输出格式为 Markdown
        db,                             // 传入数据库连接，用于获取父子类别映射
        data.records,               // 传入从月报数据中获取的时间记录
        data.total_duration,  // 传入总时长，用于计算各项百分比
        data.actual_days          // 对于月报，平均天数是该月实际有记录的天数
    );
}