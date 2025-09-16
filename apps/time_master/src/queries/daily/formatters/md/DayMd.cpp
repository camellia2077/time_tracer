// queries/daily/formatters/md/DayMd.cpp
#include "DayMd.hpp"
#include <iomanip>
#include <format>

#include "common/utils/TimeUtils.hpp"
#include "queries/shared/utils/query_utils.hpp"
#include "queries/shared/factories/TreeFmtFactory.hpp"
#include "queries/shared/Interface/ITreeFmt.hpp"
#include "queries/shared/data/DailyReportData.hpp"
#include "queries/shared/utils/BoolToString.hpp"
#include "queries/daily/formatters/md/DayMdConfig.hpp"
#include "queries/shared/utils/TimeFormat.hpp"

DayMd::DayMd(std::shared_ptr<DayMdConfig> config) : config_(config) {}

std::string DayMd::format_report(const DailyReportData& data, sqlite3* db) const {
    std::stringstream ss;
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

void DayMd::_display_header(std::stringstream& ss, const DailyReportData& data) const {
    ss << std::format("## {0} {1}\n\n", 
        config_->get_title_prefix(),
        data.date
    );
    ss << std::format("- **{0}**: {1}\n", config_->get_date_label(), data.date);
    ss << std::format("- **{0}**: {1}\n", config_->get_total_time_label(), time_format_duration(data.total_duration));
    ss << std::format("- **{0}**: {1}\n", config_->get_status_label(), bool_to_string(data.metadata.status));
    ss << std::format("- **{0}**: {1}\n", config_->get_sleep_label(), bool_to_string(data.metadata.sleep));
    ss << std::format("- **{0}**: {1}\n", config_->get_exercise_label(), bool_to_string(data.metadata.exercise));
    ss << std::format("- **{0}**: {1}\n", config_->get_getup_time_label(), data.metadata.getup_time);
    ss << std::format("- **{0}**: {1}\n", config_->get_remark_label(), data.metadata.remark);
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
        // [修改] 使用配置中的 "AllActivitiesLabel"
        ss << "\n## " << config_->get_all_activities_label() << "\n\n";
        for (const auto& record : data.detailed_records) {
            ss << std::format("- {0} - {1} ({2}): {3}\n", 
                record.start_time, 
                record.end_time,
                time_format_duration_hm(record.duration_seconds),
                record.project_path
            );
            if (record.activityRemark.has_value()) {
                ss << std::format("  - **{0}**: {1}\n", config_->get_activity_remark_label(), record.activityRemark.value());
            }
        }
        ss << "\n";
    }
}

void DayMd::_display_statistics(std::stringstream& ss, const DailyReportData& data) const {
    ss << "\n## " << config_->get_statistics_label() << "\n\n";
    ss << std::format("- **{0}**: {1}\n", 
        config_->get_sleep_time_label(), 
        time_format_duration_hm(data.sleep_time)
    );
}