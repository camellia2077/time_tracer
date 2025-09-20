// queries/shared/formatters/base/ProjectTreeFormatter.cpp
#include "ProjectTreeFormatter.hpp"
#include "queries/shared/utils/format/TimeFormat.hpp"
#include <vector>
#include <algorithm>
#include <iomanip>

namespace reporting {

// --- ProjectTreeFormatter Class Implementation ---

ProjectTreeFormatter::ProjectTreeFormatter(std::unique_ptr<IFormattingStrategy> strategy)
    : m_strategy(std::move(strategy)) {
    if (!m_strategy) {
        throw std::invalid_argument("Formatting strategy cannot be null.");
    }
}

std::string ProjectTreeFormatter::format_project_tree(
    const ProjectTree& tree,
    long long total_duration,
    int avg_days) const
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

        // 使用策略来格式化标题
        ss << m_strategy->format_category_header(
            category_name,
            time_format_duration(category_node.duration, avg_days),
            percentage
        );

        generate_sorted_output(ss, category_node, 0, avg_days);
    }

    return ss.str();
}

void ProjectTreeFormatter::generate_sorted_output(
    std::stringstream& ss,
    const ProjectNode& node,
    int indent,
    int avg_days) const
{
    std::vector<std::pair<std::string, ProjectNode>> sorted_children;
    for (const auto& pair : node.children) {
        sorted_children.push_back(pair);
    }
    std::sort(sorted_children.begin(), sorted_children.end(), [](const auto& a, const auto& b) {
        return a.second.duration > b.second.duration;
    });

    for (const auto& pair : sorted_children) {
        const std::string& name = pair.first;
        const ProjectNode& child_node = pair.second;

        if (child_node.duration > 0 || !child_node.children.empty()) {
            // 使用策略来格式化列表项
            ss << m_strategy->format_tree_node(
                name,
                time_format_duration(child_node.duration, avg_days),
                indent
            );
            generate_sorted_output(ss, child_node, indent + 1, avg_days);
        }
    }
}

} // namespace reporting