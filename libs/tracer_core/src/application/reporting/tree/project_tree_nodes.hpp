// application/reporting/tree/project_tree_nodes.hpp
#ifndef APPLICATION_REPORTING_TREE_PROJECT_TREE_NODES_H_
#define APPLICATION_REPORTING_TREE_PROJECT_TREE_NODES_H_

#include <string_view>
#include <vector>

#include "application/reporting/tree/project_tree_data.hpp"
#include "domain/reports/models/project_tree.hpp"

namespace tracer::core::application::reporting::tree {

#include "application/reporting/tree/detail/project_tree_nodes_decl.inc"

}  // namespace tracer::core::application::reporting::tree

namespace tracer_core::application::reporting::tree {

using tracer::core::application::reporting::tree::
    BuildProjectTreeNodesFromReportTree;
using tracer::core::application::reporting::tree::FindProjectTreeNodesByPath;
using tracer::core::application::reporting::tree::LimitProjectTreeDepth;

}  // namespace tracer_core::application::reporting::tree

#endif  // APPLICATION_REPORTING_TREE_PROJECT_TREE_NODES_H_
