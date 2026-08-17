// infra/query/data/data_query_types.hpp
#pragma once

#include <optional>
#include <string>
#include <string_view>

namespace tracer::core::infrastructure::query::data {

#include "infra/query/data/detail/data_query_types_decl.inc"

}  // namespace tracer::core::infrastructure::query::data

namespace tracer_core::infrastructure::query::data {

using tracer::core::infrastructure::query::data::ActivityFrequentQueryOptions;
using tracer::core::infrastructure::query::data::ActivityFrequentRow;
using tracer::core::infrastructure::query::data::DataQueryAction;
using tracer::core::infrastructure::query::data::DayDurationRow;
using tracer::core::infrastructure::query::data::DayDurationStats;
using tracer::core::infrastructure::query::data::kDefaultActivityFrequentLimit;
using tracer::core::infrastructure::query::data::
    kDefaultActivityFrequentLookbackDays;
using tracer::core::infrastructure::query::data::kSupportedDataQueryActions;

}  // namespace tracer_core::infrastructure::query::data
