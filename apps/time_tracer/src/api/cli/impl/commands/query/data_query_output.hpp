// api/cli/impl/commands/query/data_query_output.hpp
#pragma once

#include <string>
#include <vector>

#include "api/cli/impl/commands/query/data_query_types.hpp"

namespace time_tracer::cli::impl::commands::query::data {

auto PrintList(const std::vector<std::string>& items) -> void;

auto PrintDayDurations(const std::vector<DayDurationRow>& rows) -> void;

auto PrintDayDurationStats(const DayDurationStats& stats) -> void;

auto PrintTopDayDurations(const std::vector<DayDurationRow>& rows, int top_n)
    -> void;

}  // namespace time_tracer::cli::impl::commands::query::data
