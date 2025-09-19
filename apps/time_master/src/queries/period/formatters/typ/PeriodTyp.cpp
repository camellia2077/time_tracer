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

    ss << std::format(R"(#set page(margin: (top: {}cm, bottom: {}cm, left: {}cm, right: {}cm)))",
        config_->get_margin_top_cm(),
        config_->get_margin_bottom_cm(),
        config_->get_margin_left_cm(),
        config_->get_margin_right_cm()
    ) << "\n";
    
    std::string spacing_str = std::to_string(config_->get_line_spacing_em()) + "em";
    ss << std::format(R"(#set text(font: "{}", size: {}pt, spacing: {}))", 
        config_->get_base_font(),
        config_->get_base_font_size(),
        spacing_str
    ) << "\n\n";

    if (data.days_to_query <= 0) {
        ss << config_->get_invalid_days_message() << "\n";
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
        R"(#text(font: "{}", size: {}pt)[= {} {} {} ({} {} {})])",
        config_->get_title_font(),
        config_->get_report_title_font_size(),
        config_->get_title_prefix(),
        data.days_to_query,
        config_->get_title_days_unit(),
        data.start_date,
        config_->get_title_date_separator(),
        data.end_date
    );
    ss << title << "\n\n";

    if (data.actual_days > 0) {
        ss << std::format("+ *{}:* {}\n", config_->get_total_time_label(), time_format_duration(data.total_duration, data.actual_days));
        ss << std::format("+ *{}:* {}\n", config_->get_actual_days_label(), data.actual_days);
    }
}

void PeriodTyp::_display_project_breakdown(std::stringstream& ss, const PeriodReportData& data) const {
    ss << _format_project_tree(data.project_tree, data.total_duration, data.actual_days);
}

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

        ss << std::format(R"(#text(font: "{}", size: {}pt)[= {}])", 
            config_->get_category_title_font(),
            config_->get_category_title_font_size(),
            std::format("{}: {} ({:.1f}%)", 
                category_name, 
                time_format_duration(category_node.duration, avg_days), 
                percentage
            )
        ) << "\n";

        _generate_sorted_typ_output(ss, category_node, 0, avg_days);
    }

    return ss.str();
}