// common/utils/ProjectTree.hpp
#ifndef PROJECT_TREE_HPP
#define PROJECT_TREE_HPP

#include <map>
#include <string>

// 用于时间聚合的项目层级节点
struct ProjectNode {
    long long duration = 0;
    std::map<std::string, ProjectNode> children;
};

// 项目树结构的别名
using ProjectTree = std::map<std::string, ProjectNode>;

#endif // PROJECT_TREE_HPP