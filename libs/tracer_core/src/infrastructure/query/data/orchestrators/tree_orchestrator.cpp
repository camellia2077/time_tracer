// infrastructure/query/data/orchestrators/tree_orchestrator.cpp
#include "infrastructure/query/data/orchestrators/tree_orchestrator.hpp"

#include <stdexcept>
#include <utility>

#include "application/reporting/tree/project_tree_nodes.hpp"
#include "infrastructure/persistence/sqlite_data_query_service_internal.hpp"
#include "infrastructure/query/data/data_query_repository.hpp"
#include "infrastructure/query/data/renderers/data_query_renderer.hpp"

namespace query_internal =
    infrastructure::persistence::data_query_service_internal;
namespace data_query_renderers =
    tracer_core::infrastructure::query::data::renderers;

namespace tracer_core::infrastructure::query::data::orchestrators {
namespace {

constexpr int kMinUnlimitedDepth = -1;

auto BuildSuccessOutput(std::string content)
    -> tracer_core::core::dto::TextOutput {
  return {.ok = true, .content = std::move(content), .error_message = ""};
}

}  // namespace

auto HandleTreeQuery(sqlite3* db_conn,
                     const tracer_core::core::dto::DataQueryRequest& request,
                     const QueryFilters& base_filters, bool semantic_json)
    -> tracer_core::core::dto::TextOutput {
  namespace app_tree = tracer_core::application::reporting::tree;
  auto tree_filters = base_filters;
  query_internal::ApplyTreePeriod(request, db_conn, tree_filters);
  const int kMaxDepth = request.tree_max_depth.value_or(kMinUnlimitedDepth);
  if (kMaxDepth < kMinUnlimitedDepth) {
    throw std::runtime_error("--level must be >= -1.");
  }

  const auto kTree = QueryProjectTree(db_conn, tree_filters);
  const auto kNodes = app_tree::LimitProjectTreeDepth(
      app_tree::BuildProjectTreeNodesFromReportTree(kTree), kMaxDepth);
  return BuildSuccessOutput(data_query_renderers::RenderProjectTreeOutput(
      kNodes, kMaxDepth, semantic_json));
}

}  // namespace tracer_core::infrastructure::query::data::orchestrators
