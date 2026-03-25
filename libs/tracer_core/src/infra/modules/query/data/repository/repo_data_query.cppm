module;

#include "infra/query/data/data_query_repository.hpp"

export module tracer.core.infrastructure.query.data.repository
    .data_query_repository;

export namespace tracer::core::infrastructure::query::data {

using ::tracer::core::infrastructure::query::data::QueryActivitySuggestions;
using ::tracer::core::infrastructure::query::data::QueryDatesByFilters;
using ::tracer::core::infrastructure::query::data::QueryDayDurations;
using ::tracer::core::infrastructure::query::data::
    QueryDayDurationsByRootInDateRange;
using ::tracer::core::infrastructure::query::data::QueryDays;
using ::tracer::core::infrastructure::query::data::QueryLatestTrackedDate;
using ::tracer::core::infrastructure::query::data::QueryMonths;
using ::tracer::core::infrastructure::query::data::QueryProjectRootNames;
using ::tracer::core::infrastructure::query::data::QueryProjectTree;
using ::tracer::core::infrastructure::query::data::QueryYears;

}  // namespace tracer::core::infrastructure::query::data
