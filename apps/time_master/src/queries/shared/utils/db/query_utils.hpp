// queries/shared/utils/query_utils.hpp
#ifndef QUERY_UTILS_HPP
#define QUERY_UTILS_HPP

#include <sqlite3.h>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include "common/utils/ProjectTree.hpp" 
#include "queries/shared/types/ReportFormat.hpp"

// --- Standalone Utility Functions ---

// [核心修改] 移除 get_parent_map 函数，因为它所依赖的 parent_child 表已不存在。

/**
 * @brief [核心修改] generate_project_breakdown 现在直接接收记录，无需再查询父子关系。
 */
std::string generate_project_breakdown(
    ReportFormat format,
    const std::vector<std::pair<std::string, long long>>& records,
    long long total_duration,
    int avg_days
);

/**
 * @brief [核心修改] build_project_tree_from_records 不再需要 parent_map。
 */
void build_project_tree_from_records(
    ProjectTree& tree,
    const std::vector<std::pair<std::string, long long>>& records
);

// Adds or subtracts days from a date string ("YYYYMMDD").
std::string add_days_to_date_str(std::string date_str, int days);

// Gets the current system date as "YYYYMMDD".
std::string get_current_date_str();

#endif // QUERY_UTILS_HPP