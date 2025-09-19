// queries/daily/formatters/md/DayMd.cpp
#include "DayMd.hpp"
#include <iomanip>
#include <format>
#include <string>
#include <algorithm>
#include <vector>
#include "queries/shared/utils/format/BoolToString.hpp"
#include "queries/shared/utils/format/TimeFormat.hpp"
#include "queries/shared/utils/format/ReportStringUtils.hpp"

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
    // [核心修改] 调用内部方法直接格式化
    ss << _format_project_tree(data.project_tree, data.total_duration, 1);
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
}

// [新增] 从 BreakdownMd.cpp 迁移而来的逻辑
void DayMd::_generate_sorted_md_output(std::stringstream& ss, const ProjectNode& node, int indent, int avg_days) const {
    std::vector<std::pair<std::string, ProjectNode>> sorted_children;
    for (const auto& pair : node.children) {
        sorted_children.push_back(pair);
    }
    std::sort(sorted_children.begin(), sorted_children.end(), [](const auto& a, const auto& b) {
        return a.second.duration > b.second.duration;
    });

    std::string indent_str(indent * 2, ' ');

    for (const auto& pair : sorted_children) {
        const std::string& name = pair.first;
        const ProjectNode& child_node = pair.second;

        if (child_node.duration > 0 || !child_node.children.empty()) {
            ss << indent_str << "- " << name << ": " << time_format_duration(child_node.duration, avg_days) << "\n";
            _generate_sorted_md_output(ss, child_node, indent + 1, avg_days);
        }
    }
}

// [新增] 从 BreakdownMd.cpp 迁移而来的逻辑
std::string DayMd::_format_project_tree(const ProjectTree& tree, long long total_duration, int avg_days) const {
    std::stringstream ss;
    std::vector<std::pair<std::string, ProjectNode>> sorted_top_level;
    for (const auto& pair : tree) {
        sorted_top_level.push_back(pair);
    }
    std::sort(sorted_top_level.begin(), sorted_top_level.end(), [](const auto& a, const auto& b) {
        return a.second.duration > b.second.duration;
    });

    for (const auto& pair : sorted_top_level) {
        const std::string& category_name = pair.first;
        const ProjectNode& category_node = pair.second;
        double percentage = (total_duration > 0) ? (static_cast<double>(category_node.duration) / total_duration * 100.0) : 0.0;

        ss << "\n## " << category_name << ": "
           << time_format_duration(category_node.duration, avg_days)
           << " (" << std::fixed << std::setprecision(1) << percentage << "%) ##\n";

        _generate_sorted_md_output(ss, category_node, 0, avg_days);
    }

    return ss.str();
}