// queries/period/formatters/typ/PeriodTyp.cpp
#include "PeriodTyp.hpp"
#include <iomanip>
#include <format>
#include <vector>
#include <algorithm>
#include "queries/shared/utils/format/TimeFormat.hpp"     

PeriodTyp::PeriodTyp(std::shared_ptr<PeriodTypConfig> config) : config_(config) {}

std::string PeriodTyp::format_report(const PeriodReportData& data) const {
    std::stringstream ss;
    
    ss << std::format(R"(#set text(font: "{0}"))", config_->get_content_font()) << "\n\n";

    if (data.days_to_query <= 0) {
        ss << config_->get_positive_days_error() << "\n";
        return ss.str();
    }

    _display_summary(ss, data);

    if (data.actual_days == 0) {
        ss << config_->get_no_records() << "\n";
        return ss.str();
    }
    
    _display_project_breakdown(ss, data);
    return ss.str();
}

void PeriodTyp::_display_summary(std::stringstream& ss, const PeriodReportData& data) const {
    std::string title = std::format(
        R"(#text(font: "{0}", size: {1}pt)[= {2} {3} days ({4} to {5})])",
        config_->get_title_font(),
        config_->get_title_font_size(),
        config_->get_title_prefix(),
        data.days_to_query,
        data.start_date,
        data.end_date
    );
    ss << title << "\n\n";

    if (data.actual_days > 0) {
        ss << std::format("+ *{0}:* {1}\n", config_->get_total_time_label(), time_format_duration(data.total_duration, data.actual_days));
        ss << std::format("+ *{0}:* {1}\n", config_->get_actual_days_label(), data.actual_days);
    }
}

void PeriodTyp::_display_project_breakdown(std::stringstream& ss, const PeriodReportData& data) const {
    // [核心修改] 调用内部方法直接格式化
    ss << _format_project_tree(data.project_tree, data.total_duration, data.actual_days);
}

// [新增] 从 BreakdownTyp.cpp 迁移而来的逻辑
void PeriodTyp::_generate_sorted_typ_output(std::stringstream& ss, const ProjectNode& node, int indent, int avg_days) const {
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
            ss << indent_str << "+ " << name << ": " << time_format_duration(child_node.duration, avg_days) << "\n";
            _generate_sorted_typ_output(ss, child_node, indent + 1, avg_days);
        }
    }
}

// [新增] 从 BreakdownTyp.cpp 迁移而来的逻辑
std::string PeriodTyp::_format_project_tree(const ProjectTree& tree, long long total_duration, int avg_days) const {
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

        ss << "\n= " << category_name << ": "
           << time_format_duration(category_node.duration, avg_days)
           << " (" << std::fixed << std::setprecision(1) << percentage << "%)\n";

        _generate_sorted_typ_output(ss, category_node, 0, avg_days);
    }

    return ss.str();
}