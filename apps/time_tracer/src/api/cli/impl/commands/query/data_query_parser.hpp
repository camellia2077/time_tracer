// api/cli/impl/commands/query/data_query_parser.hpp
#pragma once

#include "api/cli/framework/core/arg_definitions.hpp"
#include "api/cli/impl/commands/query/data_query_types.hpp"
#include "api/cli/impl/commands/query/query_filters.hpp"

namespace time_tracer::cli::impl::commands::query::data {

[[nodiscard]] auto ParseQueryFilters(const ParsedArgs& args) -> QueryFilters;

[[nodiscard]] auto ResolveDataQueryAction(const ParsedArgs& args)
    -> DataQueryAction;

}  // namespace time_tracer::cli::impl::commands::query::data
