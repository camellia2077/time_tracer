// queries/monthly/formatters/tex/MonthTex.cpp
#include "MonthTex.hpp"
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include "queries/shared/utils/format/TimeFormat.hpp"     
#include "queries/shared/utils/tex/TexUtils.hpp"

MonthTex::MonthTex(std::shared_ptr<MonthTexConfig> config) : config_(config) {}

std::string MonthTex::format_report(const MonthlyReportData& data) const {
    if (data.year_month == "INVALID") {
        return config_->get_invalid_format_message() + "\n";
    }

    std::stringstream ss;
    ss << TexUtils::get_tex_preamble(config_->get_main_font(), config_->get_cjk_main_font());

    _display_summary(ss, data);
    if (data.actual_days == 0) {
        ss << config_->get_no_records_message() << "\n";
    } else {
        _display_project_breakdown(ss, data);
    }
    
    ss << TexUtils::get_tex_postfix();
    return ss.str();
}

void MonthTex::_display_summary(std::stringstream& ss, const MonthlyReportData& data) const {
    std::string title_month = data.year_month.substr(0, 4) + "-" + data.year_month.substr(4, 2);
    ss << "\\section*{" << config_->get_report_title() << " " << TexUtils::escape_latex(title_month) << "}\n\n";

    if (data.actual_days > 0) {
        ss << "\\begin{itemize}" << config_->get_compact_list_options() << "\n";
        ss << "    \\item \\textbf{" << config_->get_actual_days_label() << "}: " << data.actual_days << "\n";
        ss << "    \\item \\textbf{" << config_->get_total_time_label()  << "}: " << TexUtils::escape_latex(time_format_duration(data.total_duration, data.actual_days)) << "\n";
        ss << "\\end{itemize}\n\n";
    }
}

void MonthTex::_display_project_breakdown(std::stringstream& ss, const MonthlyReportData& data) const {
    // [核心修改] 调用内部方法直接格式化
    ss << _format_project_tree(data.project_tree, data.total_duration, data.actual_days);
}

// [新增] 从 BreakdownTex.cpp 迁移而来的逻辑
void MonthTex::_generate_sorted_tex_output(std::stringstream& ss, const ProjectNode& node, int avg_days) const {
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

    ss << "\\begin{itemize}[topsep=0pt, itemsep=-0.5ex]\n";

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

// [新增] 从 BreakdownTex.cpp 迁移而来的逻辑
std::string MonthTex::_format_project_tree(const ProjectTree& tree, long long total_duration, int avg_days) const {
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