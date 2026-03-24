module;

#include <memory>
#include <optional>
#include <string>
#include <vector>

class IProjectRepository;

export module tracer.core.application.query.tree.viewer;

export import tracer.core.application.query.tree.data;

export namespace tracer::core::application::query::tree {

#include "application/query/tree/detail/project_tree_viewer_decl.inc"

}  // namespace tracer::core::application::query::tree

export namespace tracer::core::application::modquery::tree {

using tracer::core::application::query::tree::ProjectTreeViewer;

}  // namespace tracer::core::application::modquery::tree
