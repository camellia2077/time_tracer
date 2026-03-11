// application/reporting/tree/project_tree_data.hpp
#ifndef APPLICATION_REPORTING_TREE_PROJECT_TREE_DATA_H_
#define APPLICATION_REPORTING_TREE_PROJECT_TREE_DATA_H_

#include <optional>
#include <string>
#include <vector>

struct ProjectTreeNode {

#include "application/reporting/tree/detail/project_tree_data_decl.inc"

};

namespace tracer::core::application::reporting::tree {

using ::ProjectTreeNode;

}  // namespace tracer::core::application::reporting::tree

#endif  // APPLICATION_REPORTING_TREE_PROJECT_TREE_DATA_H_
