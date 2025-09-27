// reports/shared/utils/format/TypUtils.cpp
#include "TypUtils.hpp"
#include <iomanip>
#include <format>
#include <vector>
#include <algorithm>
#include "reports/shared/utils/format/TimeFormat.hpp"

namespace { // 匿名命名空间

void generate_sorted_typ_output(std::stringstream& ss, const ProjectNode& node, int indent, int avg_days) {
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
            generate_sorted_typ_output(ss, child_node, indent + 1, avg_days);
        }
    }
}

} // 匿名命名空间结束

namespace TypUtils {

std::string format_project_tree(
    const ProjectTree& tree,
    long long total_duration,
    int avg_days,
    const std::string& category_title_font,
    int category_title_font_size)
{
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
            category_title_font,
            category_title_font_size,
            std::format("{}: {} ({:.1f}%)",
                category_name,
                time_format_duration(category_node.duration, avg_days),
                percentage
            )
        ) << "\n";

        generate_sorted_typ_output(ss, category_node, 0, avg_days);
    }

    return ss.str();
}

} // namespace TypUtils