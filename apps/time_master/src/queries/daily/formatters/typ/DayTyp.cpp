// queries/daily/formatters/typ/DayTyp.cpp
#include "DayTyp.hpp"
#include <iomanip>
#include <format>
#include <string>

// --- [核心修改] 替换 common_utils.hpp ---
#include "common/utils/TimeUtils.hpp" // For time_format_duration()

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
    std::string base_string = std::format("{0} - {1} ({2}): {3}",
        record.start_time,
        record.end_time,
        time_format_duration_hm(record.duration_seconds),
        record.project_path
    );

    for (const auto& pair : DayTypStrings::KeywordColors) {
        if (record.project_path.find(pair.first) != std::string::npos) {
            const std::string& color = pair.second;
            std::string final_output = std::format("+ #text({0})[{1}]", color, base_string);
            
            if (record.activityRemark.has_value()) {
                final_output += std::format("\n  + *{0}*: {1}", DayTypStrings::ActivityRemarkLabel, record.activityRemark.value());
            }
            return final_output;
        }
    }

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
            ss << _format_activity_line(record) << "\n";
        }
    }
}