// queries/shared/utils/report/ReportDataUtils.hpp
#ifndef REPORT_DATA_UTILS_HPP
#define REPORT_DATA_UTILS_HPP

#include <string>
#include <vector>
#include "common/utils/ProjectTree.hpp"
#include "queries/shared/types/ReportFormat.hpp"

/**
 * @brief 从扁平的记录数据生成项目明细报告。
 * @param format 报告的输出格式 (Markdown, LaTeX, etc.)。
 * @param records 包含项目路径和时长的记录向量。
 * @param total_duration 所有活动的总时长，用于计算百分比。
 * @param avg_days 平均天数，用于计算日均时间。
 * @return 格式化后的项目明细字符串。
 */
std::string generate_project_breakdown(
    ReportFormat format,
    const std::vector<std::pair<std::string, long long>>& records,
    long long total_duration,
    int avg_days
);

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