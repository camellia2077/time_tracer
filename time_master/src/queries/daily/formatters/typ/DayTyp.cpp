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
    // --- [核心修改] 新增 exercise 状态的显示 ---
    ss << std::format("+ *{0}:* {1}\n", DayTypStrings::ExerciseLabel, bool_to_string(data.metadata.exercise));
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
    // 1. 准备活动的基本信息字符串（不包含列表标记'+'）
    std::string base_string = std::format("{0} - {1} ({2}): {3}",
        record.start_time,
        record.end_time,
        time_format_duration_hm(record.duration_seconds),
        record.project_path
    );

    // 2. 检查是否需要应用颜色
    for (const auto& pair : DayTypStrings::KeywordColors) {
        if (record.project_path.find(pair.first) != std::string::npos) {
            // 情况A：找到关键词，需要应用颜色
            const std::string& color = pair.second;

            // 【Typst格式化关键点】
            // 为了解决备注也被染色的问题，我们将列表标记 `+` 移到 #text() 块的外部。
            // 这样，颜色只应用于活动描述文本 `base_string`，而列表结构保持不变。
            // Typst代码的结构变为： "+ #text(color)[活动描述]"
            // 这样后续的备注行就能被正确识别为子项目，且不会被上色。
            // 例如
            /*
            #text(rgb("#AAAAAA"))[
            + 12:34 - 14:03 (1h 29m): routine_toilet
                + *Activity Remark*: 先輩、好きです！ 私と付き合ってください！
            ]
            而不是
            #text(rgb("#AAAAAA"))[+ 12:34 - 14:03 (1h 29m): routine_toilet
                + *Activity Remark*: 先輩、好きです！ 私と付き合ってください！]
            */
            std::string final_output = std::format("+ #text({0})[{1}]", color, base_string);
            
            if (record.activityRemark.has_value()) {
                // 如果有备注，则在后面追加备注行，它将保持默认的黑色
                final_output += std::format("\n  + *{0}*: {1}", DayTypStrings::ActivityRemarkLabel, record.activityRemark.value());
            }
            return final_output;
        }
    }

    // 3. 情况B：未找到关键词，无需应用颜色
    // 对于无颜色的文本，Typst可以直接解析，无需特殊处理
    std::string final_output = "+ " + base_string;
    if (record.activityRemark.has_value()) {
        final_output += std::format("\n  + *{0}*: {1}", DayTypStrings::ActivityRemarkLabel, record.activityRemark.value());
    }
    
    return final_output;
}

void DayTyp::_display_detailed_activities(std::stringstream& ss, const DailyReportData& data) const {
    if (!data.detailed_records.empty()) {
        ss << "\n= " << DayTypStrings::AllActivitiesLabel << "\n\n";
        for (const auto& record : data.detailed_records) {
           // 【修正】不再在这里强制添加换行，
           // 换行的逻辑完全交给 _format_activity_line 来处理。
            ss << _format_activity_line(record) << "\n";
        }
    }
}