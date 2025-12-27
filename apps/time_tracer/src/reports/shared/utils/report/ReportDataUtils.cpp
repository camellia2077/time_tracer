// reports/shared/utils/report/ReportDataUtils.cpp
#include "ReportDataUtils.hpp"
#include "common/utils/StringUtils.hpp"

// [核心修改] generate_project_breakdown 函数已被移除

void build_project_tree_from_records(
    ProjectTree& tree,
    const std::vector<std::pair<std::string, long long>>& records)
{
    for (const auto& record : records) {
        const std::string& project_path = record.first;
        long long duration = record.second;

        std::vector<std::string> parts = split_string(project_path, '_');
        if (parts.empty()) continue;

        std::string top_level_category_key = parts[0];

        tree[top_level_category_key].duration += duration;
        ProjectNode* current_node = &tree[top_level_category_key];

        for (size_t i = 1; i < parts.size(); ++i) {
            current_node->children[parts[i]].duration += duration;
            current_node = &current_node->children[parts[i]];
        }
    }
}