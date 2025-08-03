#include "common/pch.h"
// queries/report_generators/period/formatter/period_md/PeriodMd.cpp
#include "PeriodMd.h"
#include <iomanip>

#include "common/common_utils.h"                                // 为了 ProjectTree

// --- 核心改动：包含新的依赖 ---
#include "queries/shared/query_utils.h"      // 用于 build_project_tree_from_records 和 get_parent_map
#include "queries/shared/breakdown/TreeFmtFactory.h" // 新的工厂
#include "queries/shared/breakdown/ITreeFmt.h"     // 工厂返回的接口


// format_report 方法的逻辑保持不变
std::string PeriodMd::format_report(const PeriodReportData& data, sqlite3* db) const {
    std::stringstream ss;
    if (data.days_to_query <= 0) {
        ss << "Number of days to query must be positive.\n";
        return ss.str();
    }

    _display_summary(ss, data);

    if (data.actual_days == 0) {
        ss << "No time records found in this period.\n";
        return ss.str();
    }
    
    // 调用更新后的 _display_project_breakdown
    _display_project_breakdown(ss, data, db);
    return ss.str();
}

// _display_summary 方法保持不变
void PeriodMd::_display_summary(std::stringstream& ss, const PeriodReportData& data) const {
    ss << "## Period Report: Last " << data.days_to_query << " days ("
       << data.start_date << " to " << data.end_date << ")\n\n";

    if (data.actual_days > 0) {
        ss << "- **Total Time Recorded**: " << time_format_duration(data.total_duration, data.actual_days) << "\n";
        ss << "- **Actual Days with Records**: " << data.actual_days << "\n";
    }
}

void PeriodMd::_display_project_breakdown(std::stringstream& ss, const PeriodReportData& data, sqlite3* db) const {
    // 调用统一的工具函数来生成项目明细的 Markdown 格式字符串
    ss << generate_project_breakdown(
        /*format=*/ ReportFormat::Markdown,     // 修正语法：使用 ::
        /*db=*/ db,
        /*records=*/ data.records,
        /*total_duration=*/ data.total_duration,
        /*avg_days=*/ data.actual_days          // 对于周期报告，平均天数是实际有记录的天数
    );
}