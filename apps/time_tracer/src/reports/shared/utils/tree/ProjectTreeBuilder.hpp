// reports/shared/utils/tree/ProjectTreeBuilder.hpp
#ifndef PROJECT_TREE_BUILDER_HPP
#define PROJECT_TREE_BUILDER_HPP

#include <string>
#include <vector>
#include <sqlite3.h>
#include "reports/shared/model/ProjectTree.hpp"
#include "reports/shared/api/shared_api.hpp"

// [修改] 加上命名空间前缀 reporting::

// 旧接口
REPORTS_SHARED_API void build_project_tree_from_records(
    reporting::ProjectTree& tree,
    const std::vector<std::pair<std::string, long long>>& records
);


REPORTS_SHARED_API void build_project_tree_from_ids(
    reporting::ProjectTree& tree,
    const std::vector<std::pair<long long, long long>>& id_records,
    sqlite3* db
);

#endif // PROJECT_TREE_BUILDER_HPP