module;

#include <string_view>
#include <vector>

#include "domain/reports/models/project_tree.hpp"

export module tracer.core.application.reporting.tree.nodes;

export import tracer.core.application.reporting.tree.data;

export namespace tracer::core::application::reporting::tree {

#include "application/reporting/tree/detail/project_tree_nodes_decl.inc"

}  // namespace tracer::core::application::reporting::tree

export namespace tracer::core::application::modreporting::tree {

using tracer::core::application::reporting::tree::
    BuildProjectTreeNodesFromReportTree;
using tracer::core::application::reporting::tree::FindProjectTreeNodesByPath;
using tracer::core::application::reporting::tree::LimitProjectTreeDepth;

}  // namespace tracer::core::application::modreporting::tree
