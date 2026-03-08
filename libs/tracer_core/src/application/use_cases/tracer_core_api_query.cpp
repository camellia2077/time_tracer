// application/use_cases/tracer_core_api_query.cpp
#if TT_ENABLE_CPP20_MODULES
import tracer.core.application.use_cases.helpers;
import tracer.core.application.reporting.tree.viewer;
#endif

#include <exception>
#include <utility>

#include "application/ports/i_data_query_service.hpp"
#if !TT_ENABLE_CPP20_MODULES
#include "application/reporting/tree/project_tree_viewer.hpp"
#include "application/use_cases/tracer_core_api_helpers.hpp"
#endif
#include "application/use_cases/tracer_core_api.hpp"

using namespace tracer_core::core::dto;
#if TT_ENABLE_CPP20_MODULES
using tracer::core::application::modreporting::tree::ProjectTreeViewer;
namespace core_api_helpers = tracer::core::application::modusecases::helpers;
#else
namespace core_api_helpers =
    tracer_core::application::use_cases::core_api_helpers;
#endif

auto TracerCoreApi::RunDataQuery(const DataQueryRequest& request)
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

auto TracerCoreApi::RunTreeQuery(const TreeQueryRequest& request)
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
