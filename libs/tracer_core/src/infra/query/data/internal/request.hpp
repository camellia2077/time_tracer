#ifndef INFRASTRUCTURE_QUERY_DATA_INTERNAL_REQUEST_H_
#define INFRASTRUCTURE_QUERY_DATA_INTERNAL_REQUEST_H_

#include "infra/sqlite_fwd.hpp"

#include <chrono>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

#include "application/ports/query/i_data_query_service.hpp"
#include "infra/query/data/data_query_models.hpp"
#include "infra/query/data/data_query_types.hpp"

class DBManager;

namespace tracer::core::infrastructure::query::data::internal {

#include "infra/query/data/internal/detail/request_decl.inc"

}  // namespace tracer::core::infrastructure::query::data::internal

namespace tracer::core::infrastructure::query::data::repository::internal {

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

}  // namespace tracer::core::infrastructure::query::data::repository::internal

#endif  // INFRASTRUCTURE_QUERY_DATA_INTERNAL_REQUEST_H_
