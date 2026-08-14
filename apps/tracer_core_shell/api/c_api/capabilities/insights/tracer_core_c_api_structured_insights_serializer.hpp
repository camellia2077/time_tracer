#ifndef APPS_TRACER_CORE_SHELL_API_C_API_CAPABILITIES_INSIGHTS_TRACER_CORE_C_API_STRUCTURED_INSIGHTS_SERIALIZER_HPP_
#define APPS_TRACER_CORE_SHELL_API_C_API_CAPABILITIES_INSIGHTS_TRACER_CORE_C_API_STRUCTURED_INSIGHTS_SERIALIZER_HPP_

#include <filesystem>

#include "application/dto/insights_responses.hpp"
#include "nlohmann/json.hpp"

namespace tracer_core::core::c_api::insights {

auto SerializeTemporalStructuredInsights(
    const tracer_core::core::dto::TemporalStructuredInsightsOutput& output,
    const std::filesystem::path& converter_config_toml_path)
    -> nlohmann::json;

}  // namespace tracer_core::core::c_api::insights

#endif  // APPS_TRACER_CORE_SHELL_API_C_API_CAPABILITIES_INSIGHTS_TRACER_CORE_C_API_STRUCTURED_INSIGHTS_SERIALIZER_HPP_
