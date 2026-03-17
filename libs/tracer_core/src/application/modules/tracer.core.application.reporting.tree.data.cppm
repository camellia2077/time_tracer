module;

#include "application/reporting/tree/project_tree_data.hpp"

export module tracer.core.application.reporting.tree.data;

export namespace tracer::core::application::reporting::tree {

using ::tracer::core::application::reporting::tree::ProjectTreeNode;

}  // namespace tracer::core::application::reporting::tree

export namespace tracer::core::application::modreporting::tree {

using tracer::core::application::reporting::tree::ProjectTreeNode;

}  // namespace tracer::core::application::modreporting::tree
