// application/query/tree/project_tree_viewer.hpp
#ifndef APPLICATION_QUERY_TREE_PROJECT_TREE_VIEWER_H_
#define APPLICATION_QUERY_TREE_PROJECT_TREE_VIEWER_H_

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "application/query/tree/project_tree_data.hpp"

class IProjectRepository;

namespace tracer::core::application::query::tree {

#include "application/query/tree/detail/project_tree_viewer_decl.inc"

}  // namespace tracer::core::application::query::tree

using ProjectTreeViewer =
    tracer::core::application::query::tree::ProjectTreeViewer;

#endif  // APPLICATION_QUERY_TREE_PROJECT_TREE_VIEWER_H_
