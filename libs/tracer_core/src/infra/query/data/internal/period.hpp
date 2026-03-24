#ifndef INFRASTRUCTURE_QUERY_DATA_INTERNAL_PERIOD_H_
#define INFRASTRUCTURE_QUERY_DATA_INTERNAL_PERIOD_H_

#include "infra/query/data/internal/request.hpp"

namespace tracer::core::infrastructure::query::data::internal {

#include "infra/query/data/internal/detail/period_decl.inc"

}  // namespace tracer::core::infrastructure::query::data::internal

namespace tracer::core::infrastructure::query::data::repository::internal {

using tracer::core::infrastructure::query::data::internal::ApplyTreePeriod;

}  // namespace tracer::core::infrastructure::query::data::repository::internal

#endif  // INFRASTRUCTURE_QUERY_DATA_INTERNAL_PERIOD_H_
