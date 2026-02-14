// infrastructure/reports/data/utils/project_tree_builder.hpp
#ifndef INFRASTRUCTURE_REPORTS_DATA_UTILS_PROJECT_TREE_BUILDER_H_
#define INFRASTRUCTURE_REPORTS_DATA_UTILS_PROJECT_TREE_BUILDER_H_

#include <sqlite3.h>

#include <string>
#include <vector>

#include "domain/reports/interfaces/i_project_info_provider.hpp"
#include "domain/reports/models/project_tree.hpp"

// [修改] 移除 REPORTS_DATA_API 宏，直接声明函数
void BuildProjectTreeFromRecords(
    reporting::ProjectTree& tree,
    const std::vector<std::pair<std::string, long long>>& records);

// [修改] 移除 REPORTS_DATA_API 宏，直接声明函数
void BuildProjectTreeFromIds(
    reporting::ProjectTree& tree,
    const std::vector<std::pair<long long, long long>>& id_records,
    const IProjectInfoProvider& provider);

#endif  // INFRASTRUCTURE_REPORTS_DATA_UTILS_PROJECT_TREE_BUILDER_H_