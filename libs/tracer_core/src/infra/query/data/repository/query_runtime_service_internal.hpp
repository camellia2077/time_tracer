// infra/query/data/repository/query_runtime_service_internal.hpp
#ifndef INFRASTRUCTURE_QUERY_DATA_REPOSITORY_QUERY_RUNTIME_SERVICE_INTERNAL_HPP_
#define INFRASTRUCTURE_QUERY_DATA_REPOSITORY_QUERY_RUNTIME_SERVICE_INTERNAL_HPP_

#include "infra/sqlite_fwd.hpp"
#include "infra/query/data/internal/period.hpp"
#include "infra/query/data/internal/report_mapping.hpp"

namespace tracer::core::infrastructure::query::data::repository::internal {

auto DispatchDataQueryAction(
    sqlite3* db_conn, const tracer_core::core::dto::DataQueryRequest& request,
    tracer::core::infrastructure::query::data::DataQueryAction action,
    const tracer::core::infrastructure::query::data::QueryFilters& base_filters)
    -> tracer_core::core::dto::TextOutput;

}  // namespace tracer::core::infrastructure::query::data::repository::internal

#endif  // INFRASTRUCTURE_QUERY_DATA_REPOSITORY_QUERY_RUNTIME_SERVICE_INTERNAL_HPP_
