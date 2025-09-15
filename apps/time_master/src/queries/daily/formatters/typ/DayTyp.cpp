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
#include "queries/shared/utils/TimeFormat.hpp"

DayTyp::DayTyp(std::shared_ptr<DayTypConfig> config) : config_(config) {}

std::string DayTyp::format_report(const DailyReportData& data, sqlite3* db) const {
    std::stringstream ss;
    ss << std::format(R"(#set text(font: "{0}"))", config_->get_content_font()) << "\n\n";

    _display_header(ss, data);

    if (data.total_duration == 0) {
        ss << config_->get_no_records() << "\n";
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
        config_->get_title_font(),
        config_->get_title_font_size(),
        config_->get_title_prefix(),
        data.date
    );
    ss << title << "\n\n";
    ss << std::format("+ *{0}:* {1}\n", config_->get_date_label(), data.date);
    ss << std::format("+ *{0}:* {1}\n", config_->get_total_time_label(), time_format_duration(data.total_duration));
    ss << std::format("+ *{0}:* {1}\n", config_->get_status_label(), bool_to_string(data.metadata.status));
    ss << std::format("+ *{0}:* {1}\n", config_->get_sleep_label(), bool_to_string(data.metadata.sleep));
    ss << std::format("+ *{0}:* {1}\n", config_->get_exercise_label(), bool_to_string(data.metadata.exercise));
    ss << std::format("+ *{0}:* {1}\n", config_->get_getup_time_label(), data.metadata.getup_time);
    ss << std::format("+ *{0}:* {1}\n", config_->get_remark_label(), data.metadata.remark);
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
    ss << "\n= " << config_->get_statistics_label() << "\n\n";
    ss << std::format("+ *{0}:* {1}\n", 
        config_->get_sleep_time_label(), 
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

    for (const auto& pair : config_->get_keyword_colors()) {
        if (record.project_path.find(pair.first) != std::string::npos) {
            const std::string& color = pair.second;
            std::string final_output = std::format("+ #text({0})[{1}]", color, base_string);
            
            if (record.activityRemark.has_value()) {
                final_output += std::format("\n  + *{0}*: {1}", config_->get_activity_remark_label(), record.activityRemark.value());
            }
            return final_output;
        }
    }

    std::string final_output = "+ " + base_string;
    if (record.activityRemark.has_value()) {
        final_output += std::format("\n  + *{0}*: {1}", config_->get_activity_remark_label(), record.activityRemark.value());
    }
    
    return final_output;
}

void DayTyp::_display_detailed_activities(std::stringstream& ss, const DailyReportData& data) const {
    if (!data.detailed_records.empty()) {
        ss << "\n= " << config_->get_all_activities_label() << "\n\n";
        for (const auto& record : data.detailed_records) {
            ss << _format_activity_line(record) << "\n";
        }
    }
}