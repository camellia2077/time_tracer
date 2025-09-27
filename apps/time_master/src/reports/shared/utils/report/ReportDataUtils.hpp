// reports/shared/utils/report/ReportDataUtils.hpp
#ifndef REPORT_DATA_UTILS_HPP
#define REPORT_DATA_UTILS_HPP

#include <string>
#include <vector>
#include "common/utils/ProjectTree.hpp"

/**
 * @brief 从记录中构建项目层级树。
 * @param tree 一个 ProjectTree 对象的引用，用于构建树状结构。
 * @param records 包含项目路径和时长的原始记录。
 */
void build_project_tree_from_records(
    ProjectTree& tree,
    const std::vector<std::pair<std::string, long long>>& records
);

#endif // REPORT_DATA_UTILS_HPP