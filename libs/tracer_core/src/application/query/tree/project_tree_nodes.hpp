// application/query/tree/project_tree_nodes.hpp
#ifndef APPLICATION_QUERY_TREE_PROJECT_TREE_NODES_H_
#define APPLICATION_QUERY_TREE_PROJECT_TREE_NODES_H_

#include <string_view>
#include <vector>

#include "application/query/tree/project_tree_data.hpp"
#include "domain/reports/models/project_tree.hpp"

namespace tracer::core::application::query::tree {

#include "application/query/tree/detail/project_tree_nodes_decl.inc"

}  // namespace tracer::core::application::query::tree

namespace tracer_core::application::query::tree {

using tracer::core::application::query::tree::
    BuildProjectTreeNodesFromReportTree;
using tracer::core::application::query::tree::FindProjectTreeNodesByPath;
using tracer::core::application::query::tree::LimitProjectTreeDepth;

}  // namespace tracer_core::application::query::tree

#endif  // APPLICATION_QUERY_TREE_PROJECT_TREE_NODES_H_
