#include "DayMd.hpp"
#include <iomanip>
#include <format>
#include <string>
#include <algorithm>
#include <vector>
#include "queries/shared/utils/format/BoolToString.hpp"
#include "queries/shared/utils/format/TimeFormat.hpp"
#include "queries/shared/utils/format/ReportStringUtils.hpp"
#include "queries/shared/utils/format/MarkdownUtils.hpp"
#include "queries/shared/factories/GenericFormatterFactory.hpp" // [新增] 引入新工厂
#include "queries/daily/formatters/md/DayMdConfig.hpp"
#include "queries/shared/data/DailyReportData.hpp"

// [新增] 自我注册逻辑
namespace {
    struct DayMdRegister {
        DayMdRegister() {
            GenericFormatterFactory<DailyReportData>::regist(ReportFormat::Markdown, 
                [](const AppConfig& cfg) -> std::unique_ptr<IReportFormatter<DailyReportData>> {
                    auto md_config = std::make_shared<DayMdConfig>(cfg.day_md_config_path);
                    return std::make_unique<DayMd>(md_config);
                });
        }
    };
    const DayMdRegister registrar;
}

DayMd::DayMd(std::shared_ptr<DayMdConfig> config) : config_(config) {}

std::string DayMd::format_report(const DailyReportData& data) const {
    std::stringstream ss;
    _display_header(ss, data);

    if (data.total_duration == 0) {
        ss << config_->get_no_records() << "\n";
        return ss.str();
    }
    
    _display_statistics(ss, data);
    _display_detailed_activities(ss, data);
    _display_project_breakdown(ss, data);
    return ss.str();
}

void DayMd::_display_header(std::stringstream& ss, const DailyReportData& data) const {
    ss << std::format("## {0} {1}\n\n", config_->get_title_prefix(), data.date);
    ss << std::format("- **{0}**: {1}\n", config_->get_date_label(), data.date);
    ss << std::format("- **{0}**: {1}\n", config_->get_total_time_label(), time_format_duration(data.total_duration));
    ss << std::format("- **{0}**: {1}\n", config_->get_status_label(), bool_to_string(data.metadata.status));
    ss << std::format("- **{0}**: {1}\n", config_->get_sleep_label(), bool_to_string(data.metadata.sleep));
    ss << std::format("- **{0}**: {1}\n", config_->get_exercise_label(), bool_to_string(data.metadata.exercise));
    ss << std::format("- **{0}**: {1}\n", config_->get_getup_time_label(), data.metadata.getup_time);
    ss << std::format("- **{0}**: {1}\n", config_->get_remark_label(), data.metadata.remark);
}

void DayMd::_display_project_breakdown(std::stringstream& ss, const DailyReportData& data) const {
    ss << MarkdownUtils::format_project_tree(data.project_tree, data.total_duration, 1);
}

void DayMd::_display_detailed_activities(std::stringstream& ss, const DailyReportData& data) const {
    if (!data.detailed_records.empty()) {
        ss << "\n## " << config_->get_all_activities_label() << "\n\n";
        for (const auto& record : data.detailed_records) {
            std::string project_path = replace_all(record.project_path, "_", config_->get_activity_connector());
            ss << std::format("- {0} - {1} ({2}): {3}\n", 
                record.start_time, 
                record.end_time,
                time_format_duration(record.duration_seconds),
                project_path
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
        time_format_duration(data.sleep_time)
    );
    ss << std::format("- **{0}**: {1}\n", 
        config_->get_anaerobic_time_label(), 
        time_format_duration(data.anaerobic_time)
    );
    ss << std::format("- **{0}**: {1}\n", 
        config_->get_cardio_time_label(), 
        time_format_duration(data.cardio_time)
    );
    ss << std::format("- **{0}**: {1}\n", 
        config_->get_grooming_time_label(), 
        time_format_duration(data.grooming_time)
    );
}