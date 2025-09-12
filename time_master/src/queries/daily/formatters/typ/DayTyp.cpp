#include "DayTyp.hpp"
#include <iomanip>
#include <format>
#include <string>

#include "common/common_utils.hpp"
#include "queries/shared/utils/query_utils.hpp"
#include "queries/shared/utils/BoolToString.hpp"
#include "queries/shared/data/DailyReportData.hpp"
#include "DayTypStrings.hpp"
#include "queries/shared/utils/TimeFormat.hpp"

// 其他函数（format_report, _display_header 等）保持不变

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

std::string DayTyp::_format_activity_line(const TimeRecord& record) const {
    // 活動の基本情報を一行で定義
    std::string base_string = std::format("{0} - {1} ({2}): {3}",
        record.start_time,
        record.end_time,
        time_format_duration_hm(record.duration_seconds),
        record.project_path
    );

    // キーワードに基づいて基本情報行を色付け
    std::string colorized_base;
    bool color_found = false;
    for (const auto& pair : DayTypStrings::KeywordColors) {
        const std::string& keyword = pair.first;
        const std::string& color = pair.second;
        
        if (record.project_path.find(keyword) != std::string::npos) {
            colorized_base = std::format("#text({0})[+ {1}]", color, base_string);
            color_found = true;
            break;
        }
    }
    if (!color_found) {
        colorized_base = "+ " + base_string;
    }

    // 注釈が存在する場合、ネストされたリスト項目として追加
    if (record.activityRemark.has_value()) {
        std::string remark_str = std::format("\n  + _({0}: {1})_", DayTypStrings::ActivityRemarkLabel, record.activityRemark.value());
        return colorized_base + remark_str;
    }

    return colorized_base;
}

void DayTyp::_display_detailed_activities(std::stringstream& ss, const DailyReportData& data) const {
    if (!data.detailed_records.empty()) {
        ss << "\n= " << DayTypStrings::AllActivitiesLabel << "\n\n";
        for (const auto& record : data.detailed_records) {
            // 【修正】ここで無条件に改行を追加するのをやめ、
            // _format_activity_lineに改行の責務を完全に委譲する
            ss << _format_activity_line(record) << "\n";
        }
    }
}