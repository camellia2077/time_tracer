// queries/daily/formatters/typ/DayTyp.cpp
#include "DayTyp.hpp"
#include <iomanip>
#include <format>
#include <string>
#include <algorithm>
#include <vector>
#include "queries/shared/utils/format/BoolToString.hpp"
#include "queries/shared/utils/format/TimeFormat.hpp"
#include "queries/shared/utils/format/ReportStringUtils.hpp"

DayTyp::DayTyp(std::shared_ptr<DayTypConfig> config) : config_(config) {}

std::string DayTyp::format_report(const DailyReportData& data) const {
    std::stringstream ss;
    std::string spacing_str = std::to_string(config_->get_line_spacing_em()) + "em";
    ss << std::format(R"(#set text(font: "{0}", size: {1}pt, spacing: {2}))", 
        config_->get_base_font(), 
        config_->get_base_font_size(),
        spacing_str) << "\n\n";

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

void DayTyp::_display_header(std::stringstream& ss, const DailyReportData& data) const {
    std::string title = std::format(
        R"(#text(font: "{0}", size: {1}pt)[= {2} {3}])",
        config_->get_title_font(),
        config_->get_report_title_font_size(),
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

void DayTyp::_display_project_breakdown(std::stringstream& ss, const DailyReportData& data) const {
    ss << _format_project_tree(data.project_tree, data.total_duration, 1);
}

void DayTyp::_display_statistics(std::stringstream& ss, const DailyReportData& data) const {
    ss << std::format(R"(#text(size: {}pt)[= {}])", 
        config_->get_category_title_font_size(), 
        config_->get_statistics_label()) << "\n\n";
    ss << std::format("+ *{0}:* {1}\n", 
        config_->get_sleep_time_label(), 
        time_format_duration(data.sleep_time)
    );
}

std::string DayTyp::_format_activity_line(const TimeRecord& record) const {
    std::string project_path = replace_all(record.project_path, "_", config_->get_activity_connector());
    std::string base_string = std::format("{0} - {1} ({2}): {3}",
        record.start_time,
        record.end_time,
        time_format_duration(record.duration_seconds),
        project_path
    );

    for (const auto& pair : config_->get_keyword_colors()) {
        if (record.project_path.find(pair.first) != std::string::npos) {
            const std::string& hex_color = pair.second;
            std::string typst_color_format = std::format(R"(rgb("{}"))", hex_color);
            std::string final_output = std::format("+ #text({0})[{1}]", typst_color_format, base_string);
            
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
        ss << std::format(R"(#text(size: {}pt)[= {}])", 
            config_->get_category_title_font_size(), 
            config_->get_all_activities_label()) << "\n\n";
        for (const auto& record : data.detailed_records) {
            ss << _format_activity_line(record) << "\n";
        }
    }
}

void DayTyp::_generate_sorted_typ_output(std::stringstream& ss, const ProjectNode& node, int indent, int avg_days) const {
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

std::string DayTyp::_format_project_tree(const ProjectTree& tree, long long total_duration, int avg_days) const {
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

        ss << std::format(R"(#text(size: {}pt)[= {}])", 
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