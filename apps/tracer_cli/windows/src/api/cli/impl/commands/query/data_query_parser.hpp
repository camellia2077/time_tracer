// api/cli/impl/commands/query/data_query_parser.hpp
#pragma once

#include "api/cli/framework/core/arg_definitions.hpp"
#include "application/dto/core_requests.hpp"

namespace tracer_core::cli::impl::commands::query::data {

[[nodiscard]] auto ParseDataQueryRequest(const ParsedArgs &args)
    -> tracer_core::core::dto::DataQueryRequest;

} // namespace tracer_core::cli::impl::commands::query::data
