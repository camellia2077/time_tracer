// api/cli/impl/commands/query/data_query_parser.hpp
#pragma once

#include "api/cli/framework/core/arg_definitions.hpp"
#include "application/dto/core_requests.hpp"

namespace time_tracer::cli::impl::commands::query::data {

[[nodiscard]] auto ParseDataQueryRequest(const ParsedArgs &args)
    -> time_tracer::core::dto::DataQueryRequest;

} // namespace time_tracer::cli::impl::commands::query::data
