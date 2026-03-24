#include "application/use_cases/query_api.hpp"

#include <stdexcept>
#include <utility>

#include "application/use_cases/core_api_failure.hpp"

import tracer.core.application.query.tree.viewer;

namespace tracer::core::application::use_cases {

using namespace tracer_core::core::dto;
namespace core_api_failure = tracer::core::application::use_cases::failure;
using tracer::core::application::query::tree::ProjectTreeViewer;

QueryApi::QueryApi(ProjectRepositoryPtr project_repository,
                   DataQueryServicePtr data_query_service)
    : project_repository_(std::move(project_repository)),
      data_query_service_(std::move(data_query_service)) {
  if (!project_repository_) {
    throw std::invalid_argument("project_repository must not be null.");
  }
  if (!data_query_service_) {
    throw std::invalid_argument("data_query_service must not be null.");
  }
}

auto QueryApi::RunDataQuery(const DataQueryRequest& request) -> TextOutput {
  try {
    auto response = data_query_service_->RunDataQuery(request);
    if (!response.ok && response.error_message.empty()) {
      response.error_message = core_api_failure::BuildErrorMessage(
          "RunDataQuery", "Data query service returned a failed response.");
    }
    return response;
  } catch (const std::exception& exception) {
    return core_api_failure::BuildTextFailure("RunDataQuery", exception);
  } catch (...) {
    return core_api_failure::BuildTextFailure("RunDataQuery");
  }
}

auto QueryApi::RunTreeQuery(const TreeQueryRequest& request)
    -> TreeQueryResponse {
  try {
    ProjectTreeViewer viewer(project_repository_);

    if (request.list_roots) {
      return {.ok = true,
              .found = true,
              .roots = viewer.GetRoots(),
              .tree = {},
              .error_message = ""};
    }

    const auto tree_result =
        viewer.GetTree(request.root_pattern, request.max_depth);
    if (!tree_result.has_value()) {
      return {.ok = true,
              .found = false,
              .roots = {},
              .tree = {},
              .error_message = ""};
    }

    return {.ok = true,
            .found = true,
            .roots = {},
            .tree = TreeQueryPayload{.nodes = std::move(*tree_result)},
            .error_message = ""};
  } catch (const std::exception& exception) {
    return core_api_failure::BuildTreeFailure("RunTreeQuery", exception);
  } catch (...) {
    return core_api_failure::BuildTreeFailure("RunTreeQuery");
  }
}

}  // namespace tracer::core::application::use_cases
