// application/dto/tree_query_response.hpp
#ifndef APPLICATION_DTO_TREE_QUERY_RESPONSE_H_
#define APPLICATION_DTO_TREE_QUERY_RESPONSE_H_

#include <string>
#include <vector>

#include "application/reporting/tree/project_tree_data.hpp"

namespace tracer_core::core::dto {

struct TreeQueryPayload {
  std::vector<ProjectTreeNode> nodes;
};

struct TreeQueryResponse {
  bool ok = true;
  bool found = true;
  std::vector<std::string> roots;
  TreeQueryPayload tree;
  std::string error_message;
};

}  // namespace tracer_core::core::dto

#endif  // APPLICATION_DTO_TREE_QUERY_RESPONSE_H_
