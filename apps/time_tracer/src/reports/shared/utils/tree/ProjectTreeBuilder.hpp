// reports/shared/utils/tree/ProjectTreeBuilder.hpp
#ifndef PROJECT_TREE_BUILDER_HPP
#define PROJECT_TREE_BUILDER_HPP

#include <string>
#include <vector>
#include <sqlite3.h>
#include "reports/shared/model/ProjectTree.hpp"
#include "reports/shared/api/shared_api.hpp"
#include "reports/shared/interfaces/IProjectInfoProvider.hpp" // [新增]

REPORTS_SHARED_API void build_project_tree_from_records(
    reporting::ProjectTree& tree,
    const std::vector<std::pair<std::string, long long>>& records
);

// [修改] 接口变动：移除 sqlite3*, 增加 IProjectInfoProvider&
REPORTS_SHARED_API void build_project_tree_from_ids(
    reporting::ProjectTree& tree,
    const std::vector<std::pair<long long, long long>>& id_records,
    const IProjectInfoProvider& provider // [修改]
);

#endif // PROJECT_TREE_BUILDER_HPP