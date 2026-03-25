// application/query/tree/project_tree_data.hpp
#ifndef APPLICATION_QUERY_TREE_PROJECT_TREE_DATA_H_
#define APPLICATION_QUERY_TREE_PROJECT_TREE_DATA_H_

#include <optional>
#include <string>
#include <vector>

namespace tracer::core::application::query::tree {

struct ProjectTreeNode {
  std::string name;
  std::string path;
  std::optional<long long> duration_seconds;
  std::vector<ProjectTreeNode> children;
};

}  // namespace tracer::core::application::query::tree

using ProjectTreeNode = tracer::core::application::query::tree::ProjectTreeNode;

#endif  // APPLICATION_QUERY_TREE_PROJECT_TREE_DATA_H_
