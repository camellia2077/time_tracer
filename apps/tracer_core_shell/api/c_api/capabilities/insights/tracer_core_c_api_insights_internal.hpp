#ifndef APPS_TRACER_CORE_SHELL_API_C_API_CAPABILITIES_INSIGHTS_TRACER_CORE_C_API_INSIGHTS_INTERNAL_HPP_
#define APPS_TRACER_CORE_SHELL_API_C_API_CAPABILITIES_INSIGHTS_TRACER_CORE_C_API_INSIGHTS_INTERNAL_HPP_

#include "application/dto/shared_envelopes.hpp"

namespace tracer_core::core::c_api::insights {

auto BuildInsightsTextResponse(const tracer_core::core::dto::TextOutput& output)
    -> const char*;

}  // namespace tracer_core::core::c_api::insights

#endif  // APPS_TRACER_CORE_SHELL_API_C_API_CAPABILITIES_INSIGHTS_TRACER_CORE_C_API_INSIGHTS_INTERNAL_HPP_
