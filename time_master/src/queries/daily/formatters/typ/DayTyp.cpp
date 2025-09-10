#include "DayTyp.hpp"
#include <iomanip>
#include <format>

#include "common/common_utils.hpp"
#include "queries/shared/utils/query_utils.hpp"
#include "queries/shared/utils/BoolToString.hpp" // [新增] 引入新的工具头文件
#include "queries/shared/data/DailyReportData.hpp"
#include "DayTypStrings.hpp" // 唯一且专属的配置文件
#include "queries/shared/utils/TimeFormat.hpp" // [新增] 引入新的头文件

std::string DayTyp::format_report(const DailyReportData& data, sqlite3* db) const {
    std::stringstream ss;
    
    // (修改) 为占位符加上编号 {0}
    ss << std::format(R"(#set text(font: "{0}"))", DayTypStrings::ContentFont) << "\n\n";

    _display_header(ss, data);

    if (data.total_duration == 0) {
        ss << DayTypStrings::NoRecords << "\n";
        return ss.str();
    }
    
    _display_statistics(ss, data);
    _display_detailed_activities(ss, data);

    _display_project_breakdown(ss, data, db);
    return ss.str();
}

void DayTyp::_display_header(std::stringstream& ss, const DailyReportData& data) const {
    // (修改) 为标题的占位符加上编号 {0}, {1}, {2}, {3}
    std::string title = std::format(
        R"(#text(font: "{0}", size: {1}pt)[= {2} {3}])",
        DayTypStrings::TitleFont,        // {0}
        DayTypStrings::TitleFontSize,    // {1}
        DayTypStrings::TitlePrefix,      // {2}
        data.date                        // {3}
    );
    ss << title << "\n\n";
    
    // (修改) 为其余部分的占位符加上编号 {0}, {1}
    ss << std::format("+ *{0}:* {1}\n", DayTypStrings::DateLabel, data.date);
    ss << std::format("+ *{0}:* {1}\n", DayTypStrings::TotalTimeLabel, time_format_duration(data.total_duration));
    ss << std::format("+ *{0}:* {1}\n", DayTypStrings::StatusLabel, bool_to_string(data.metadata.status)); // [修改] 调用新的工具函数
    ss << std::format("+ *{0}:* {1}\n", DayTypStrings::SleepLabel, bool_to_string(data.metadata.sleep)); // [新增] 添加 Sleep 字段 
    ss << std::format("+ *{0}:* {1}\n", DayTypStrings::GetupTimeLabel, data.metadata.getup_time);
    ss << std::format("+ *{0}:* {1}\n", DayTypStrings::RemarkLabel, data.metadata.remark);
}

void DayTyp::_display_project_breakdown(std::stringstream& ss, const DailyReportData& data, sqlite3* db) const {
    ss << generate_project_breakdown(
        ReportFormat::Typ, 
        db, 
        data.records, 
        data.total_duration, 
        1
    );
}

void DayTyp::_display_statistics(std::stringstream& ss, const DailyReportData& data) const {
    ss << "\n= " << DayTypStrings::StatisticsLabel << "\n\n";
    ss << std::format("+ *{0}:* {1}\n", 
        DayTypStrings::SleepTimeLabel, 
        time_format_duration_hm(data.sleep_time)
    );
}

void DayTyp::_display_detailed_activities(std::stringstream& ss, const DailyReportData& data) const {
    if (!data.detailed_records.empty()) {
        ss << "\n= " << DayTypStrings::AllActivitiesLabel << "\n\n";
        for (const auto& record : data.detailed_records) {
            ss << std::format("+ {0} - {1} ({2}): {3}\n", 
                record.start_time, 
                record.end_time,
                time_format_duration_hm(record.duration_seconds),
                record.project_path
            );
        }
    }
}