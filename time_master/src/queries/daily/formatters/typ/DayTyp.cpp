#include "DayTyp.hpp"
#include <iomanip>
#include <format>
#include <string> // 引入 <string> 以使用 std::string::npos

#include "common/common_utils.hpp"
#include "queries/shared/utils/query_utils.hpp"
#include "queries/shared/utils/BoolToString.hpp"
#include "queries/shared/data/DailyReportData.hpp"
#include "DayTypStrings.hpp"
#include "queries/shared/utils/TimeFormat.hpp"

// format_report, _display_header, _display_project_breakdown, _display_statistics 函数保持不变...
// (为简洁起见，这里省略了未改动的函数)

std::string DayTyp::format_report(const DailyReportData& data, sqlite3* db) const {
    std::stringstream ss;
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
    std::string title = std::format(
        R"(#text(font: "{0}", size: {1}pt)[= {2} {3}])",
        DayTypStrings::TitleFont,
        DayTypStrings::TitleFontSize,
        DayTypStrings::TitlePrefix,
        data.date
    );
    ss << title << "\n\n";
    ss << std::format("+ *{0}:* {1}\n", DayTypStrings::DateLabel, data.date);
    ss << std::format("+ *{0}:* {1}\n", DayTypStrings::TotalTimeLabel, time_format_duration(data.total_duration));
    ss << std::format("+ *{0}:* {1}\n", DayTypStrings::StatusLabel, bool_to_string(data.metadata.status));
    ss << std::format("+ *{0}:* {1}\n", DayTypStrings::SleepLabel, bool_to_string(data.metadata.sleep));
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

// [修改] 新增的辅助函数实现
std::string DayTyp::_format_activity_line(const TimeRecord& record) const {
    // 基础的活动内容字符串
    std::string base_string = std::format("{0} - {1} ({2}): {3}",
        record.start_time,
        record.end_time,
        time_format_duration_hm(record.duration_seconds),
        record.project_path
    );

    // 检查 project_path 是否包含 "study"
    if (record.project_path.find("study") != std::string::npos) {
        // 如果包含，则用绿色文本格式化输出
        return std::format("#text(green)[+ {0}]", base_string);
    } else {
        // 否则，使用默认格式输出
        return "+ " + base_string;
    }
}

// [修改] _display_detailed_activities 调用新的辅助函数
void DayTyp::_display_detailed_activities(std::stringstream& ss, const DailyReportData& data) const {
    if (!data.detailed_records.empty()) {
        ss << "\n= " << DayTypStrings::AllActivitiesLabel << "\n\n";
        for (const auto& record : data.detailed_records) {
            ss << _format_activity_line(record) << "\n";
        }
    }
}