// reports/shared/utils/report/ReportDataUtils.cpp
#include "ReportDataUtils.hpp"
#include "common/utils/StringUtils.hpp"
#include "ProjectNameCache.hpp"


void build_project_tree_from_records(
    reporting::ProjectTree& tree, 
    const std::vector<std::pair<std::string, long long>>& records)
{
    for (const auto& record : records) {
        const std::string& project_path = record.first;
        long long duration = record.second;

        std::vector<std::string> parts = split_string(project_path, '_');
        if (parts.empty()) continue;

        std::string top_level_category_key = parts[0];
        tree[top_level_category_key].duration += duration;
        
        // [修改] 局部变量类型 ProjectNode -> reporting::ProjectNode
        reporting::ProjectNode* current_node = &tree[top_level_category_key];

        for (size_t i = 1; i < parts.size(); ++i) {
            current_node->children[parts[i]].duration += duration;
            current_node = &current_node->children[parts[i]];
        }
    }
}

void build_project_tree_from_ids(
    reporting::ProjectTree& tree, // [修改] 参数类型
    const std::vector<std::pair<long long, long long>>& id_records,
    sqlite3* db)
{
    // 1. 确保缓存加载
    auto& cache = ProjectNameCache::instance();
    cache.ensure_loaded(db);

    // 2. 遍历聚合后的 ID 记录
    for (const auto& record : id_records) {
        long long project_id = record.first;
        long long duration = record.second;

        std::vector<std::string> parts = cache.get_path_parts(project_id);
        if (parts.empty()) continue;

        // 3. 构建树
        std::string top_level_category_key = parts[0];
        tree[top_level_category_key].duration += duration;
        
        // [修改] 局部变量类型 ProjectNode -> reporting::ProjectNode
        reporting::ProjectNode* current_node = &tree[top_level_category_key];

        for (size_t i = 1; i < parts.size(); ++i) {
            current_node->children[parts[i]].duration += duration;
            current_node = &current_node->children[parts[i]];
        }
    }
}