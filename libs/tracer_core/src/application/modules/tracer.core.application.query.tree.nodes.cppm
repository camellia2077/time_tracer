module;

#include <string_view>
#include <vector>

#include "domain/reports/models/project_tree.hpp"

export module tracer.core.application.query.tree.nodes;

export import tracer.core.application.query.tree.data;

export namespace tracer::core::application::query::tree {

#include "application/query/tree/detail/project_tree_nodes_decl.inc"

}  // namespace tracer::core::application::query::tree

export namespace tracer::core::application::modquery::tree {

using tracer::core::application::query::tree::
    BuildProjectTreeNodesFromReportTree;
using tracer::core::application::query::tree::FindProjectTreeNodesByPath;
using tracer::core::application::query::tree::LimitProjectTreeDepth;

}  // namespace tracer::core::application::modquery::tree
