// queries/shared/utils/report/ReportDataUtils.cpp
#include "ReportDataUtils.hpp"
#include "common/utils/StringUtils.hpp"
#include "queries/shared/factories/TreeFmtFactory.hpp"
#include "queries/shared/interface/ITreeFmt.hpp"

std::string generate_project_breakdown(
    ReportFormat format,
    const std::vector<std::pair<std::string, long long>>& records,
    long long total_duration,
    int avg_days)
{
    ProjectTree project_tree;
    build_project_tree_from_records(project_tree, records);

    auto formatter = TreeFmtFactory::createFormatter(format);

    if (formatter) {
        return formatter->format(project_tree, total_duration, avg_days);
    }
    
    return ""; 
}

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