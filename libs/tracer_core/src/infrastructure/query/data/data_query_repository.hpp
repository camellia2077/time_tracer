// infrastructure/query/data/data_query_repository.hpp
#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "domain/reports/models/project_tree.hpp"
#include "infrastructure/query/data/data_query_models.hpp"
#include "infrastructure/query/data/data_query_types.hpp"

struct sqlite3;

namespace tracer::core::infrastructure::query::data {

#include "infrastructure/query/data/detail/data_query_repository_decl.inc"

}  // namespace tracer::core::infrastructure::query::data

namespace tracer_core::infrastructure::query::data {

using tracer::core::infrastructure::query::data::QueryActivitySuggestions;
using tracer::core::infrastructure::query::data::QueryDatesByFilters;
using tracer::core::infrastructure::query::data::QueryDayDurations;
using tracer::core::infrastructure::query::data::
    QueryDayDurationsByRootInDateRange;
using tracer::core::infrastructure::query::data::QueryDays;
using tracer::core::infrastructure::query::data::QueryLatestTrackedDate;
using tracer::core::infrastructure::query::data::QueryMonths;
using tracer::core::infrastructure::query::data::QueryProjectRootNames;
using tracer::core::infrastructure::query::data::QueryProjectTree;
using tracer::core::infrastructure::query::data::QueryYears;

}  // namespace tracer_core::infrastructure::query::data
