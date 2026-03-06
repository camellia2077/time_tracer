// application/reporting/tree/project_tree_nodes.hpp
#ifndef APPLICATION_REPORTING_TREE_PROJECT_TREE_NODES_H_
#define APPLICATION_REPORTING_TREE_PROJECT_TREE_NODES_H_

#include <string_view>
#include <vector>

#include "application/reporting/tree/project_tree_data.hpp"
#include "domain/reports/models/project_tree.hpp"

namespace tracer_core::application::reporting::tree {

[[nodiscard]] auto BuildProjectTreeNodesFromReportTree(
    const ::reporting::ProjectTree& tree) -> std::vector<ProjectTreeNode>;

[[nodiscard]] auto FindProjectTreeNodesByPath(
    const std::vector<ProjectTreeNode>& roots, std::string_view root_pattern)
    -> std::vector<ProjectTreeNode>;

[[nodiscard]] auto LimitProjectTreeDepth(
    const std::vector<ProjectTreeNode>& roots, int max_depth)
    -> std::vector<ProjectTreeNode>;

}  // namespace tracer_core::application::reporting::tree

#endif  // APPLICATION_REPORTING_TREE_PROJECT_TREE_NODES_H_
