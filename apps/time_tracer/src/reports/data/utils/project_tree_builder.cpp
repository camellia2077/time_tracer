// reports/data/utils/project_tree_builder.cpp
#include "project_tree_builder.hpp"
#include "common/utils/string_utils.hpp"

void build_project_tree_from_records(
    reporting::ProjectTree& tree, 
    const std::vector<std::pair<std::string, long long>>& records)
{
    // ... 保持不变 ...
    for (const auto& record : records) {
        const std::string& project_path = record.first;
        long long duration = record.second;

        std::vector<std::string> parts = split_string(project_path, '_');
        if (parts.empty()) continue;

        std::string top_level_category_key = parts[0];
        tree[top_level_category_key].duration += duration;
        
        reporting::ProjectNode* current_node = &tree[top_level_category_key];

        for (size_t i = 1; i < parts.size(); ++i) {
            current_node->children[parts[i]].duration += duration;
            current_node = &current_node->children[parts[i]];
        }
    }
}

// [修改] 实现变更
void build_project_tree_from_ids(
    reporting::ProjectTree& tree,
    const std::vector<std::pair<long long, long long>>& id_records,
    const IProjectInfoProvider& provider) // [修改] 参数
{
    // 1. 移除 ensure_loaded 调用，假设传入的 provider 已经是可用的
    // 或者由调用者(Service)负责 ensure_loaded

    // 2. 遍历聚合后的 ID 记录
    for (const auto& record : id_records) {
        long long project_id = record.first;
        long long duration = record.second;

        // [修改] 使用 provider 接口调用
        std::vector<std::string> parts = provider.get_path_parts(project_id);
        if (parts.empty()) continue;

        // 3. 构建树
        std::string top_level_category_key = parts[0];
        tree[top_level_category_key].duration += duration;
        
        reporting::ProjectNode* current_node = &tree[top_level_category_key];

        for (size_t i = 1; i < parts.size(); ++i) {
            current_node->children[parts[i]].duration += duration;
            current_node = &current_node->children[parts[i]];
        }
    }
}