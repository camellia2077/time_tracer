// reports/data/utils/project_tree_builder.hpp
#ifndef REPORTS_DATA_UTILS_PROJECT_TREE_BUILDER_H_
#define REPORTS_DATA_UTILS_PROJECT_TREE_BUILDER_H_

#include <sqlite3.h>

#include <string>
#include <vector>

#include "reports/data/interfaces/i_project_info_provider.hpp"
#include "reports/data/model/project_tree.hpp"

// [修改] 移除 REPORTS_DATA_API 宏，直接声明函数
void build_project_tree_from_records(
    reporting::ProjectTree& tree,
    const std::vector<std::pair<std::string, long long>>& records);

// [修改] 移除 REPORTS_DATA_API 宏，直接声明函数
void build_project_tree_from_ids(
    reporting::ProjectTree& tree,
    const std::vector<std::pair<long long, long long>>& id_records,
    const IProjectInfoProvider& provider);

#endif  // REPORTS_DATA_UTILS_PROJECT_TREE_BUILDER_H_