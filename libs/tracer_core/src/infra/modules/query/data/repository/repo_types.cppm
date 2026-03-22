module;

#include "infra/query/data/data_query_types.hpp"

export module tracer.core.infrastructure.query.data.repository.types;

export namespace tracer::core::infrastructure::query::data {

using ::tracer::core::infrastructure::query::data::ActivitySuggestionQueryOptions;
using ::tracer::core::infrastructure::query::data::ActivitySuggestionRow;
using ::tracer::core::infrastructure::query::data::DataQueryAction;
using ::tracer::core::infrastructure::query::data::DayDurationRow;
using ::tracer::core::infrastructure::query::data::DayDurationStats;
using ::tracer::core::infrastructure::query::data::kDefaultActivitySuggestionLimit;
using ::tracer::core::infrastructure::query::data::
    kDefaultActivitySuggestionLookbackDays;
using ::tracer::core::infrastructure::query::data::kSupportedDataQueryActions;

}  // namespace tracer::core::infrastructure::query::data
