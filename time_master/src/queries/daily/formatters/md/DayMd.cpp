
#include "DayMd.h"
#include <iomanip>

#include "common/common_utils.h"                      // 为了 ProjectTree

// --- 核心改动：包含新的依赖 ---
#include "queries/shared/query_utils.h"   // 用于 build_project_tree_from_records 和 get_parent_map
#include "queries/shared/breakdown/TreeFmtFactory.h" // 新的工厂
#include "queries/shared/breakdown/ITreeFmt.h"     // 工厂返回的接口
#include "queries/shared/DailyReportData.h"


// format_report 方法保持不变，因为它将工作委托给了辅助函数
std::string DayMd::format_report(const DailyReportData& data, sqlite3* db) const {
    std::stringstream ss;
    _display_header(ss, data);

    if (data.total_duration == 0) {
        ss << "No time records for this day.\n";
        return ss.str();
    }
    
    // 调用更新后的 _display_project_breakdown
    _display_project_breakdown(ss, data, db);
    return ss.str();
}

// _display_header 方法保持不变
void DayMd::_display_header(std::stringstream& ss, const DailyReportData& data) const {
    ss << "## Daily Report for " << data.date << "\n\n"; 
    ss << "- **Date**: " << data.date << "\n";
    ss << "- **Total Time Recorded**: " << time_format_duration(data.total_duration) << "\n";
    ss << "- **Status**: " << data.metadata.status << "\n";
    ss << "- **Getup Time**: " << data.metadata.getup_time << "\n";
    ss << "- **Remark**:" << data.metadata.remark << "\n";
}

// --- 核心改动：_display_project_breakdown 的全新实现 ---
void DayMd::_display_project_breakdown(std::stringstream& ss, const DailyReportData& data, sqlite3* db) const {
    ss << generate_project_breakdown(
        ReportFormat::Markdown, // 指定输出格式为 Markdown
        db, // 传入数据库连接，用于获取父子类别映射
        data.records, // 传入从日报数据中获取的时间记录
        data.total_duration,  // 传入总时长，用于计算各项百分比
        1 // 日报的 avg_days 总是 1
    );
}