#ifndef PROJECT_TREE_HPP
#define PROJECT_TREE_HPP

#include <unordered_map> // [修改] 引入 unordered_map
#include <string>


struct ProjectNode {
    long long duration = 0;
    // [优化] 使用 unordered_map 提升构建速度
    // 在生成报告时（如 ProjectTreeFormatter），
    // 程序显式地按 duration（时长）进行了重新排序。
    // 这意味着 std::map 在构建时的排序工作是完全浪费的。
    // 改用 std::unordered_map
    // 插入和查找的平均时间复杂度从 O(log N) 降低到 O(1)。
    std::unordered_map<std::string, ProjectNode> children; 
};

using ProjectTree = std::unordered_map<std::string, ProjectNode>;

#endif // PROJECT_TREE_HPP