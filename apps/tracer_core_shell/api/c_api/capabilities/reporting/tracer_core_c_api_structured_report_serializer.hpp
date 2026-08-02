#ifndef APPS_TRACER_CORE_SHELL_API_C_API_CAPABILITIES_REPORTING_TRACER_CORE_C_API_STRUCTURED_REPORT_SERIALIZER_HPP_
#define APPS_TRACER_CORE_SHELL_API_C_API_CAPABILITIES_REPORTING_TRACER_CORE_C_API_STRUCTURED_REPORT_SERIALIZER_HPP_

#include "application/dto/reporting_responses.hpp"
#include "nlohmann/json.hpp"

namespace tracer_core::core::c_api::reporting {

auto SerializeTemporalStructuredReport(
    const tracer_core::core::dto::TemporalStructuredReportOutput& output)
    -> nlohmann::json;

}  // namespace tracer_core::core::c_api::reporting

#endif  // APPS_TRACER_CORE_SHELL_API_C_API_CAPABILITIES_REPORTING_TRACER_CORE_C_API_STRUCTURED_REPORT_SERIALIZER_HPP_
