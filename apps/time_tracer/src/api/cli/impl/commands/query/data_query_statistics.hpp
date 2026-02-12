// api/cli/impl/commands/query/data_query_statistics.hpp
#pragma once

#include <vector>

#include "api/cli/impl/commands/query/data_query_types.hpp"

namespace time_tracer::cli::impl::commands::query::data {

[[nodiscard]] auto ComputeDayDurationStats(
    const std::vector<DayDurationRow>& rows) -> DayDurationStats;

}  // namespace time_tracer::cli::impl::commands::query::data
