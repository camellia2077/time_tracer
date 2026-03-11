// infrastructure/query/data/data_query_types.hpp
#pragma once

#include <optional>
#include <string>
#include <string_view>

namespace tracer::core::infrastructure::query::data {

#include "infrastructure/query/data/detail/data_query_types_decl.inc"

}  // namespace tracer::core::infrastructure::query::data

namespace tracer_core::infrastructure::query::data {

using tracer::core::infrastructure::query::data::ActivitySuggestionQueryOptions;
using tracer::core::infrastructure::query::data::ActivitySuggestionRow;
using tracer::core::infrastructure::query::data::DataQueryAction;
using tracer::core::infrastructure::query::data::DayDurationRow;
using tracer::core::infrastructure::query::data::DayDurationStats;
using tracer::core::infrastructure::query::data::
    kDefaultActivitySuggestionLimit;
using tracer::core::infrastructure::query::data::
    kDefaultActivitySuggestionLookbackDays;
using tracer::core::infrastructure::query::data::kSupportedDataQueryActions;

}  // namespace tracer_core::infrastructure::query::data
