// queries/shared/utils/query_utils.cpp
#include "query_utils.hpp"
#include "common/utils/StringUtils.hpp"
#include "queries/shared/factories/TreeFmtFactory.hpp" 
#include "queries/shared/Interface/ITreeFmt.hpp"       
#include "common/utils/ProjectTree.hpp" 

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <map>
#include <algorithm>
#include <chrono>
#include <ctime>

std::string generate_project_breakdown(
    ReportFormat format,
    const std::vector<std::pair<std::string, long long>>& records,
    long long total_duration,
    int avg_days)
{
    ProjectTree project_tree;
    build_project_tree_from_records(project_tree, records);

    auto formatter = TreeFmtFactory::createFormatter(format);

    if (formatter) {
        return formatter->format(project_tree, total_duration, avg_days);
    }
    
    return ""; 
}

void build_project_tree_from_records(
    ProjectTree& tree,
    const std::vector<std::pair<std::string, long long>>& records)
{
    for (const auto& record : records) {
        const std::string& project_path = record.first;
        long long duration = record.second;

        std::vector<std::string> parts = split_string(project_path, '_');
        if (parts.empty()) continue;

        std::string top_level_category_key = parts[0];

        tree[top_level_category_key].duration += duration;
        ProjectNode* current_node = &tree[top_level_category_key];

        for (size_t i = 1; i < parts.size(); ++i) {
            current_node->children[parts[i]].duration += duration;
            current_node = &current_node->children[parts[i]];
        }
    }
}

std::string add_days_to_date_str(std::string date_str, int days) {
    int year = std::stoi(date_str.substr(0, 4));
    int month = std::stoi(date_str.substr(4, 2));
    int day = std::stoi(date_str.substr(6, 2));

    std::tm t{};
    t.tm_year = year - 1900;
    t.tm_mon = month - 1;
    t.tm_mday = day + days;
    std::mktime(&t);

    std::stringstream ss;
    ss << std::put_time(&t, "%Y%m%d");
    return ss.str();
}

std::string get_current_date_str() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y%m%d");
    return ss.str();
}