#ifndef INFRASTRUCTURE_QUERY_DATA_INTERNAL_REQUEST_H_
#define INFRASTRUCTURE_QUERY_DATA_INTERNAL_REQUEST_H_

#include "infrastructure/sqlite_fwd.hpp"

#include <chrono>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

#include "application/ports/i_data_query_service.hpp"
#include "infrastructure/query/data/data_query_models.hpp"
#include "infrastructure/query/data/data_query_types.hpp"

class DBManager;

namespace tracer::core::infrastructure::query::data::internal {

#include "infrastructure/query/data/internal/detail/request_decl.inc"

}  // namespace tracer::core::infrastructure::query::data::internal

namespace infrastructure::persistence::data_query_service_internal {

using tracer::core::infrastructure::query::data::internal::BuildCliFilters;
using tracer::core::infrastructure::query::data::internal::EnsureDbConnectionOrThrow;
using tracer::core::infrastructure::query::data::internal::FormatIsoDate;
using tracer::core::infrastructure::query::data::internal::NormalizeBoundaryDate;
using tracer::core::infrastructure::query::data::internal::NormalizeProjectRootFilter;
using tracer::core::infrastructure::query::data::internal::ParseIsoDate;
using tracer::core::infrastructure::query::data::internal::ResolveCurrentSystemLocalDate;
using tracer::core::infrastructure::query::data::internal::ResolvePositiveLookbackDays;
using tracer::core::infrastructure::query::data::internal::ToCliDataQueryAction;
using tracer::core::infrastructure::query::data::internal::TrimCopy;

}  // namespace infrastructure::persistence::data_query_service_internal

#endif  // INFRASTRUCTURE_QUERY_DATA_INTERNAL_REQUEST_H_
