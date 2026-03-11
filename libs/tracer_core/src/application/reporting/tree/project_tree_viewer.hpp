// application/reporting/tree/project_tree_viewer.hpp
#ifndef APPLICATION_REPORTING_TREE_PROJECT_TREE_VIEWER_H_
#define APPLICATION_REPORTING_TREE_PROJECT_TREE_VIEWER_H_

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "application/reporting/tree/project_tree_data.hpp"

class IProjectRepository;

namespace tracer::core::application::reporting::tree {

#include "application/reporting/tree/detail/project_tree_viewer_decl.inc"

}  // namespace tracer::core::application::reporting::tree

using ProjectTreeViewer =
    tracer::core::application::reporting::tree::ProjectTreeViewer;

#endif  // APPLICATION_REPORTING_TREE_PROJECT_TREE_VIEWER_H_
