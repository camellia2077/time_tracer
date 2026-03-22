// infra/query/data/orchestrators/date_range_resolver.hpp
#pragma once

#include <optional>
#include <string>
#include <string_view>

namespace tracer::core::infrastructure::query::data::orchestrators {

#include "infra/query/data/orchestrators/detail/date_range_resolver_decl.inc"

}  // namespace tracer::core::infrastructure::query::data::orchestrators

namespace tracer_core::infrastructure::query::data::orchestrators {

using tracer::core::infrastructure::query::data::orchestrators::
    DateRangeBoundaries;
using tracer::core::infrastructure::query::data::orchestrators::
    DateRangeValidationErrors;
using tracer::core::infrastructure::query::data::orchestrators::
    ExplicitDateRangeErrors;
using tracer::core::infrastructure::query::data::orchestrators::
    ResolvedDateRange;
using tracer::core::infrastructure::query::data::orchestrators::
    ResolveExplicitDateRange;
using tracer::core::infrastructure::query::data::orchestrators::
    ResolveRollingDateRange;
using tracer::core::infrastructure::query::data::orchestrators::
    ValidateDateRange;

}  // namespace tracer_core::infrastructure::query::data::orchestrators
