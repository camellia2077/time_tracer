// application/use_cases/time_tracer_core_api_query.cpp
#include <exception>
#include <utility>

#include "application/ports/i_data_query_service.hpp"
#include "application/reporting/tree/project_tree_viewer.hpp"
#include "application/use_cases/time_tracer_core_api.hpp"
#include "application/use_cases/time_tracer_core_api_helpers.hpp"

using namespace time_tracer::core::dto;
namespace core_api_helpers =
    time_tracer::application::use_cases::core_api_helpers;

auto TimeTracerCoreApi::RunDataQuery(const DataQueryRequest& request)
    -> TextOutput {
  try {
    auto response = data_query_service_->RunDataQuery(request);
    if (!response.ok && response.error_message.empty()) {
      response.error_message = core_api_helpers::BuildErrorMessage(
          "RunDataQuery", "Data query service returned a failed response.");
    }
    return response;
  } catch (const std::exception& exception) {
    return core_api_helpers::BuildTextFailure("RunDataQuery", exception);
  } catch (...) {
    return core_api_helpers::BuildTextFailure("RunDataQuery");
  }
}

auto TimeTracerCoreApi::RunTreeQuery(const TreeQueryRequest& request)
    -> TreeQueryResponse {
  try {
    ProjectTreeViewer viewer(project_repository_);

    if (request.list_roots) {
      return {.ok = true,
              .found = true,
              .roots = viewer.GetRoots(),
              .nodes = {},
              .error_message = ""};
    }

    const auto kTreeResult =
        viewer.GetTree(request.root_pattern, request.max_depth);
    if (!kTreeResult.has_value()) {
      return {.ok = true,
              .found = false,
              .roots = {},
              .nodes = {},
              .error_message = ""};
    }

    return {.ok = true,
            .found = true,
            .roots = {},
            .nodes = std::move(*kTreeResult),
            .error_message = ""};
  } catch (const std::exception& exception) {
    return core_api_helpers::BuildTreeFailure("RunTreeQuery", exception);
  } catch (...) {
    return core_api_helpers::BuildTreeFailure("RunTreeQuery");
  }
}
