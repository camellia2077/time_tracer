#include "DayMd.hpp"
#include <iomanip>

#include "common/common_utils.hpp" // For ProjectTree

#include "queries/shared/utils/query_utils.hpp"   // 用于 build_project_tree_from_records 和 get_parent_map
#include "queries/shared/factories/TreeFmtFactory.hpp" // 新的工厂
#include "queries/shared/Interface/ITreeFmt.hpp"     // 工厂返回的接口
#include "queries/shared/data/DailyReportData.hpp"
#include "queries/shared/utils/BoolToString.hpp" // 调用bool转字符串工具

std::string DayMd::format_report(const DailyReportData& data, sqlite3* db) const {
    std::stringstream ss;
    _display_header(ss, data);

    if (data.total_duration == 0) {
        ss << "No time records for this day.\n";
        return ss.str();
    }
    
    // 调用 _display_project_breakdown
    _display_project_breakdown(ss, data, db);
    return ss.str();
}
void DayMd::_display_header(std::stringstream& ss, const DailyReportData& data) const {
    ss << "## Daily Report for " << data.date << "\n\n"; 
    ss << "- **Date**: " << data.date << "\n";
    ss << "- **Total Time Recorded**: " << time_format_duration(data.total_duration) << "\n";
    ss << "- **Status**: " << bool_to_string(data.metadata.status) << "\n"; // [修改] 调用新的工具函数
    ss << "- **Sleep**: " << bool_to_string(data.metadata.sleep) << "\n"; // [新增] 添加 Sleep 字段并转换其值
    ss << "- **Getup Time**: " << data.metadata.getup_time << "\n";
    ss << "- **Remark**:" << data.metadata.remark << "\n";
}

void DayMd::_display_project_breakdown(std::stringstream& ss, const DailyReportData& data, sqlite3* db) const {
    ss << generate_project_breakdown(
        ReportFormat::Markdown, // 指定输出格式为 Markdown
        db, // 传入数据库连接，用于获取父子类别映射
        data.records, // 传入从日报数据中获取的时间记录
        data.total_duration,  // 传入总时长，用于计算各项百分比
        1 // 日报的 avg_days 总是 1
    );
}