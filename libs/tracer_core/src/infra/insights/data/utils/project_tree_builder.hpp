// infra/insights/data/utils/project_tree_builder.hpp
#ifndef INFRASTRUCTURE_INSIGHTS_DATA_UTILS_PROJECT_TREE_BUILDER_H_
#define INFRASTRUCTURE_INSIGHTS_DATA_UTILS_PROJECT_TREE_BUILDER_H_

#include <cstdint>
#include <string>
#include <vector>

#include "domain/insights/interfaces/i_project_info_provider.hpp"
#include "domain/insights/models/project_tree.hpp"

// [修改] 移除 INSIGHTS_DATA_API 宏，直接声明函数
void BuildProjectTreeFromRecords(
    insights::ProjectTree& tree,
    const std::vector<std::pair<std::string, std::int64_t>>& records);

// [修改] 移除 INSIGHTS_DATA_API 宏，直接声明函数
void BuildProjectTreeFromIds(
    insights::ProjectTree& tree,
    const std::vector<std::pair<std::int64_t, std::int64_t>>& id_records,
    const IProjectInfoProvider& provider);

#endif  // INFRASTRUCTURE_INSIGHTS_DATA_UTILS_PROJECT_TREE_BUILDER_H_
