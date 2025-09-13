// queries/daily/formatters/md/DayMd.cpp
#include "DayMd.hpp"
#include <iomanip>
#include <format>

// --- [核心修改] 替换 common_utils.hpp ---
#include "common/utils/TimeUtils.hpp" // For time_format_duration()

#include "queries/shared/utils/query_utils.hpp"
#include "queries/shared/factories/TreeFmtFactory.hpp"
#include "queries/shared/Interface/ITreeFmt.hpp"
#include "queries/shared/data/DailyReportData.hpp"
#include "queries/shared/utils/BoolToString.hpp"
#include "queries/daily/formatters/md/DayMdStrings.hpp"
#include "queries/shared/utils/TimeFormat.hpp"

std::string DayMd::format_report(const DailyReportData& data, sqlite3* db) const {
    std::stringstream ss;
    _display_header(ss, data);

    if (data.total_duration == 0) {
        ss << DayMdStrings::NoRecords << "\n";
        return ss.str();
    }
    
    _display_statistics(ss, data);
    _display_detailed_activities(ss, data);
    
    _display_project_breakdown(ss, data, db);
    return ss.str();
}

void DayMd::_display_header(std::stringstream& ss, const DailyReportData& data) const {
    ss << std::format("## {0} {1}\n\n", 
        DayMdStrings::TitlePrefix,
        data.date
    );
    ss << std::format("- **{0}**: {1}\n", DayMdStrings::DateLabel, data.date);
    ss << std::format("- **{0}**: {1}\n", DayMdStrings::TotalTimeLabel, time_format_duration(data.total_duration));
    ss << std::format("- **{0}**: {1}\n", DayMdStrings::StatusLabel, bool_to_string(data.metadata.status));
    ss << std::format("- **{0}**: {1}\n", DayMdStrings::SleepLabel, bool_to_string(data.metadata.sleep));
    ss << std::format("- **{0}**: {1}\n", DayMdStrings::ExerciseLabel, bool_to_string(data.metadata.exercise));
    ss << std::format("- **{0}**: {1}\n", DayMdStrings::GetupTimeLabel, data.metadata.getup_time);
    ss << std::format("- **{0}**: {1}\n", DayMdStrings::RemarkLabel, data.metadata.remark);
}


void DayMd::_display_project_breakdown(std::stringstream& ss, const DailyReportData& data, sqlite3* db) const {
    ss << generate_project_breakdown(
        ReportFormat::Markdown,
        db,
        data.records,
        data.total_duration,
        1
    );
}

void DayMd::_display_detailed_activities(std::stringstream& ss, const DailyReportData& data) const {
    if (!data.detailed_records.empty()) {
        ss << "\n## All Activities\n\n";
        for (const auto& record : data.detailed_records) {
            ss << std::format("- {0} - {1} ({2}): {3}\n", 
                record.start_time, 
                record.end_time,
                time_format_duration_hm(record.duration_seconds),
                record.project_path
            );
            if (record.activityRemark.has_value()) {
                ss << std::format("  - **{0}**: {1}\n", DayMdStrings::ActivityRemarkLabel, record.activityRemark.value());
            }
        }
        ss << "\n";
    }
}

void DayMd::_display_statistics(std::stringstream& ss, const DailyReportData& data) const {
    ss << "\n## " << DayMdStrings::StatisticsLabel << "\n\n";
    ss << std::format("- **{0}**: {1}\n", 
        DayMdStrings::SleepTimeLabel, 
        time_format_duration_hm(data.sleep_time)
    );
}