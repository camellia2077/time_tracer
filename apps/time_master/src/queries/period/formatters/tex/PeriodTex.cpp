// queries/period/formatters/tex/PeriodTex.cpp
#include "PeriodTex.hpp"
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <format>
#include "queries/shared/utils/format/TimeFormat.hpp"    
#include "queries/shared/utils/tex/TexUtils.hpp"

PeriodTex::PeriodTex(std::shared_ptr<PeriodTexConfig> config) : config_(config) {}

std::string PeriodTex::format_report(const PeriodReportData& data) const {
    if (data.days_to_query <= 0) {
        return config_->get_invalid_days_message() + "\n";
    }

    std::stringstream ss;
    ss << TexUtils::get_tex_preamble(
        config_->get_main_font(), 
        config_->get_cjk_main_font(),
        config_->get_font_size(),
        config_->get_margin_in()
    );
    
    _display_summary(ss, data);
    if (data.actual_days == 0) {
        ss << config_->get_no_records_message() << "\n";
    } else {
        _display_project_breakdown(ss, data);
    }

    ss << TexUtils::get_tex_postfix();
    return ss.str();
}

void PeriodTex::_display_summary(std::stringstream& ss, const PeriodReportData& data) const {
    ss << "\\section*{"
       << config_->get_report_title_prefix() << " " << data.days_to_query << " "
       << config_->get_report_title_days() << " ("
       << TexUtils::escape_latex(data.start_date) << " " << config_->get_report_title_date_separator() << " "
       << TexUtils::escape_latex(data.end_date) << ")}\n\n";

    if (data.actual_days > 0) {
        std::string compact_list_options = std::format("[topsep={}pt, itemsep={}ex]", 
            config_->get_list_top_sep_pt(), 
            config_->get_list_item_sep_ex()
        );
        ss << "\\begin{itemize}" << compact_list_options << "\n";
        ss << "    \\item \\textbf{" << config_->get_total_time_label() << "}: "
           << TexUtils::escape_latex(time_format_duration(data.total_duration, data.actual_days)) << "\n";
        ss << "    \\item \\textbf{" << config_->get_actual_days_label() << "}: "
           << data.actual_days << "\n";
        ss << "\\end{itemize}\n\n";
    }
}

void PeriodTex::_display_project_breakdown(std::stringstream& ss, const PeriodReportData& data) const {
    ss << _format_project_tree(data.project_tree, data.total_duration, data.actual_days);
}

void PeriodTex::_generate_sorted_tex_output(std::stringstream& ss, const ProjectNode& node, int avg_days) const {
    if (node.children.empty()) {
        return;
    }

    std::vector<std::pair<std::string, ProjectNode>> sorted_children;
    for (const auto& pair : node.children) {
        sorted_children.push_back(pair);
    }
    std::sort(sorted_children.begin(), sorted_children.end(), [](const auto& a, const auto& b) {
        return a.second.duration > b.second.duration;
    });

    std::string itemize_options = std::format("[topsep={}pt, itemsep={}ex]",
        config_->get_list_top_sep_pt(),
        config_->get_list_item_sep_ex()
    );
    ss << "\\begin{itemize}" << itemize_options << "\n";

    for (const auto& pair : sorted_children) {
        const std::string& name = pair.first;
        const ProjectNode& child_node = pair.second;

        if (child_node.duration > 0 || !child_node.children.empty()) {
            ss << "    \\item " << TexUtils::escape_latex(name) << ": "
               << TexUtils::escape_latex(time_format_duration(child_node.duration, avg_days));

            if (!child_node.children.empty()) {
                ss << "\n";
                _generate_sorted_tex_output(ss, child_node, avg_days);
            }
            ss << "\n";
        }
    }

    ss << "\\end{itemize}\n";
}

std::string PeriodTex::_format_project_tree(const ProjectTree& tree, long long total_duration, int avg_days) const {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1);

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

        ss << "\\section*{" << TexUtils::escape_latex(category_name) << ": "
           << TexUtils::escape_latex(time_format_duration(category_node.duration, avg_days))
           << " (" << percentage << "\\%)}\n";

        _generate_sorted_tex_output(ss, category_node, avg_days);
        ss << "\n";
    }

    return ss.str();
}