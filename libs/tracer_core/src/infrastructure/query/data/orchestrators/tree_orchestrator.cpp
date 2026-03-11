// infrastructure/query/data/orchestrators/tree_orchestrator.cpp
#include "infrastructure/query/data/orchestrators/tree_orchestrator.hpp"

#include <stdexcept>
#include <utility>

#include "application/reporting/tree/project_tree_nodes.hpp"
#if TT_ENABLE_CPP20_MODULES
import tracer.core.infrastructure.query.data.internal.period;
import tracer.core.infrastructure.query.data.repository;
import tracer.core.infrastructure.query.data.renderers;
#endif
#if !TT_ENABLE_CPP20_MODULES
#include "infrastructure/query/data/internal/period.hpp"
#include "infrastructure/query/data/data_query_repository.hpp"
#include "infrastructure/query/data/renderers/data_query_renderer.hpp"
#endif

namespace app_tree = tracer::core::application::reporting::tree;
namespace query_internal =
    tracer::core::infrastructure::query::data::internal;
namespace query_data_repository = tracer::core::infrastructure::query::data;
namespace data_query_renderers =
    tracer::core::infrastructure::query::data::renderers;

namespace tracer::core::infrastructure::query::data::orchestrators {
namespace {

constexpr int kMinUnlimitedDepth = -1;

auto BuildSuccessOutput(std::string content)
    -> tracer_core::core::dto::TextOutput {
  return {.ok = true, .content = std::move(content), .error_message = ""};
}

}  // namespace

auto HandleTreeQuery(sqlite3* db_conn,
                     const tracer_core::core::dto::DataQueryRequest& request,
                     const QueryFilters& base_filters,
                     tracer_core::core::dto::DataQueryOutputMode output_mode)
    -> tracer_core::core::dto::TextOutput {
  auto tree_filters = base_filters;
  query_internal::ApplyTreePeriod(request, db_conn, tree_filters);
  const int kMaxDepth = request.tree_max_depth.value_or(kMinUnlimitedDepth);
  if (kMaxDepth < kMinUnlimitedDepth) {
    throw std::runtime_error("--level must be >= -1.");
  }

  const auto kTree =
      query_data_repository::QueryProjectTree(db_conn, tree_filters);
  const auto kNodes = app_tree::LimitProjectTreeDepth(
      app_tree::BuildProjectTreeNodesFromReportTree(kTree), kMaxDepth);
  return BuildSuccessOutput(data_query_renderers::RenderProjectTreeOutput(
      kNodes, kMaxDepth, output_mode));
}

}  // namespace tracer::core::infrastructure::query::data::orchestrators
