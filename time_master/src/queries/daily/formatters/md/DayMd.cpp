#include "DayMd.hpp"
#include <iomanip>
#include <format> // [新增] 引入 format 头文件

#include "common/common_utils.hpp" // For ProjectTree

#include "queries/shared/utils/query_utils.hpp"   // 用于 build_project_tree_from_records 和 get_parent_map
#include "queries/shared/factories/TreeFmtFactory.hpp" // 新的工厂
#include "queries/shared/Interface/ITreeFmt.hpp"     // 工厂返回的接口
#include "queries/shared/data/DailyReportData.hpp"
#include "queries/shared/utils/BoolToString.hpp" // 调用bool转字符串工具
#include "DayMdStrings.hpp" // 引入文本配置文件

std::string DayMd::format_report(const DailyReportData& data, sqlite3* db) const {
    std::stringstream ss;
    _display_header(ss, data);

    if (data.total_duration == 0) {
        ss << DayMdStrings::NoRecords << "\n"; // [修改]
        return ss.str();
    }
    
    // 调用 _display_project_breakdown
    _display_project_breakdown(ss, data, db);
    return ss.str();
}
void DayMd::_display_header(std::stringstream& ss, const DailyReportData& data) const {
    ss << std::format("## {0} {1}\n\n", 
        DayMdStrings::TitlePrefix, // {0}
        data.date // {1}
    );
    ss << std::format("- **{0}**: {1}\n", DayMdStrings::DateLabel, data.date);
    ss << std::format("- **{0}**: {1}\n", DayMdStrings::TotalTimeLabel, time_format_duration(data.total_duration));
    ss << std::format("- **{0}**: {1}\n", DayMdStrings::StatusLabel, bool_to_string(data.metadata.status));
    ss << std::format("- **{0}**: {1}\n", DayMdStrings::SleepLabel, bool_to_string(data.metadata.sleep));
    ss << std::format("- **{0}**: {1}\n", DayMdStrings::GetupTimeLabel, data.metadata.getup_time);
    ss << std::format("- **{0}**: {1}\n", DayMdStrings::RemarkLabel, data.metadata.remark);
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