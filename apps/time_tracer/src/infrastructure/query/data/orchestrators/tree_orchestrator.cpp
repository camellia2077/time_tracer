#include "infrastructure/query/data/orchestrators/tree_orchestrator.hpp"

#include <stdexcept>
#include <utility>

#include "infrastructure/persistence/sqlite_data_query_service_internal.hpp"
#include "infrastructure/query/data/data_query_repository.hpp"
#include "infrastructure/query/data/renderers/data_query_renderer.hpp"

namespace query_internal =
    infrastructure::persistence::data_query_service_internal;
namespace data_query_renderers =
    time_tracer::infrastructure::query::data::renderers;

namespace time_tracer::infrastructure::query::data::orchestrators {
namespace {

constexpr int kMinUnlimitedDepth = -1;

auto BuildSuccessOutput(std::string content)
    -> time_tracer::core::dto::TextOutput {
  return {.ok = true, .content = std::move(content), .error_message = ""};
}

}  // namespace

auto HandleTreeQuery(sqlite3* db_conn,
                     const time_tracer::core::dto::DataQueryRequest& request,
                     const QueryFilters& base_filters, bool semantic_json)
    -> time_tracer::core::dto::TextOutput {
  auto tree_filters = base_filters;
  query_internal::ApplyTreePeriod(request, db_conn, tree_filters);
  const int kMaxDepth = request.tree_max_depth.value_or(kMinUnlimitedDepth);
  if (kMaxDepth < kMinUnlimitedDepth) {
    throw std::runtime_error("--level must be >= -1.");
  }

  const auto kTree = QueryProjectTree(db_conn, tree_filters);
  return BuildSuccessOutput(data_query_renderers::RenderProjectTreeOutput(
      kTree, kMaxDepth, semantic_json));
}

}  // namespace time_tracer::infrastructure::query::data::orchestrators
