module;

#include "infra/query/data/data_query_types.hpp"

export module tracer.core.infrastructure.query.data.repository.types;

export namespace tracer::core::infrastructure::query::data {

using ::tracer::core::infrastructure::query::data::
    ActivityFrequentQueryOptions;
using ::tracer::core::infrastructure::query::data::ActivityFrequentRow;
using ::tracer::core::infrastructure::query::data::DataQueryAction;
using ::tracer::core::infrastructure::query::data::DayDurationRow;
using ::tracer::core::infrastructure::query::data::DayDurationStats;
using ::tracer::core::infrastructure::query::data::
    kDefaultActivityFrequentLimit;
using ::tracer::core::infrastructure::query::data::
    kDefaultActivityFrequentLookbackDays;
using ::tracer::core::infrastructure::query::data::kSupportedDataQueryActions;

}  // namespace tracer::core::infrastructure::query::data
