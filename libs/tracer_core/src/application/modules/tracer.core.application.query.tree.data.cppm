module;

#include "application/query/tree/project_tree_data.hpp"

export module tracer.core.application.query.tree.data;

export namespace tracer::core::application::query::tree {

using ::tracer::core::application::query::tree::ProjectTreeNode;

}  // namespace tracer::core::application::query::tree

export namespace tracer::core::application::modquery::tree {

using tracer::core::application::query::tree::ProjectTreeNode;

}  // namespace tracer::core::application::modquery::tree
