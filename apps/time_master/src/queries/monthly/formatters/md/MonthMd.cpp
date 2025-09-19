// queries/monthly/formatters/md/MonthMd.cpp
#include "MonthMd.hpp"
#include <iomanip>
#include <format>
#include <vector>
#include <algorithm>
#include "queries/shared/utils/format/TimeFormat.hpp"

MonthMd::MonthMd(std::shared_ptr<MonthMdConfig> config) : config_(config) {}

std::string MonthMd::format_report(const MonthlyReportData& data) const {
    std::stringstream ss;
    if (data.year_month == "INVALID") {
        ss << config_->get_invalid_format_message() << "\n";
        return ss.str();
    }

    _display_summary(ss, data);

    if (data.actual_days == 0) {
        ss << config_->get_no_records_message() << "\n";
        return ss.str();
    }
    
    _display_project_breakdown(ss, data);
    return ss.str();
}

void MonthMd::_display_summary(std::stringstream& ss, const MonthlyReportData& data) const {
    ss << std::format("## {0} {1}-{2}\n\n", 
        config_->get_report_title(), 
        data.year_month.substr(0, 4), 
        data.year_month.substr(4, 2)
    );

    if (data.actual_days > 0) {
        ss << std::format("- **{0}**: {1}\n", config_->get_actual_days_label(), data.actual_days);
        ss << std::format("- **{0}**: {1}\n", config_->get_total_time_label(), time_format_duration(data.total_duration, data.actual_days));
    }
}

void MonthMd::_display_project_breakdown(std::stringstream& ss, const MonthlyReportData& data) const {
    // [核心修改] 调用内部方法直接格式化
    ss << _format_project_tree(data.project_tree, data.total_duration, data.actual_days);
}

// [新增] 从 BreakdownMd.cpp 迁移而来的逻辑
void MonthMd::_generate_sorted_md_output(std::stringstream& ss, const ProjectNode& node, int indent, int avg_days) const {
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
std::string MonthMd::_format_project_tree(const ProjectTree& tree, long long total_duration, int avg_days) const {
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