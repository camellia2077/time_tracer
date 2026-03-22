// infra/persistence/sqlite_data_query_service_internal.hpp
#ifndef INFRASTRUCTURE_PERSISTENCE_SQLITE_DATA_QUERY_SERVICE_INTERNAL_HPP_
#define INFRASTRUCTURE_PERSISTENCE_SQLITE_DATA_QUERY_SERVICE_INTERNAL_HPP_

#include "infra/sqlite_fwd.hpp"
#include "infra/query/data/internal/period.hpp"
#include "infra/query/data/internal/report_mapping.hpp"

namespace infrastructure::persistence::data_query_service_internal {

auto DispatchDataQueryAction(
    sqlite3* db_conn, const tracer_core::core::dto::DataQueryRequest& request,
    tracer::core::infrastructure::query::data::DataQueryAction action,
    const tracer::core::infrastructure::query::data::QueryFilters& base_filters)
    -> tracer_core::core::dto::TextOutput;

}  // namespace infrastructure::persistence::data_query_service_internal

#endif  // INFRASTRUCTURE_PERSISTENCE_SQLITE_DATA_QUERY_SERVICE_INTERNAL_HPP_

