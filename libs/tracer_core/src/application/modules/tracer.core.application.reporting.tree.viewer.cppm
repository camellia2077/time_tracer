module;

#include <memory>
#include <optional>
#include <string>
#include <vector>

class IProjectRepository;

export module tracer.core.application.reporting.tree.viewer;

export import tracer.core.application.reporting.tree.data;

export namespace tracer::core::application::reporting::tree {

#include "application/reporting/tree/detail/project_tree_viewer_decl.inc"

}  // namespace tracer::core::application::reporting::tree

export namespace tracer::core::application::modreporting::tree {

using tracer::core::application::reporting::tree::ProjectTreeViewer;

}  // namespace tracer::core::application::modreporting::tree
