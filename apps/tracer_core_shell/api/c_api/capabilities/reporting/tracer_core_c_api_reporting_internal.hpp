#ifndef APPS_TRACER_CORE_SHELL_API_C_API_CAPABILITIES_REPORTING_TRACER_CORE_C_API_REPORTING_INTERNAL_HPP_
#define APPS_TRACER_CORE_SHELL_API_C_API_CAPABILITIES_REPORTING_TRACER_CORE_C_API_REPORTING_INTERNAL_HPP_

#include <filesystem>

namespace tracer::core::application::use_cases {
class ITracerCoreRuntime;
}

namespace tracer_core::core::dto {
struct ReportExportRequest;
struct TextOutput;
}  // namespace tracer_core::core::dto

namespace tracer_core::core::c_api::reporting {

auto BuildReportTextResponse(const tracer_core::core::dto::TextOutput& output)
    -> const char*;

void ExportSpecificLegacyCompatibility(
    tracer::core::application::use_cases::ITracerCoreRuntime& runtime,
    const std::filesystem::path& export_root,
    const tracer_core::core::dto::ReportExportRequest& request);

void ExportAllLegacyCompatibility(
    tracer::core::application::use_cases::ITracerCoreRuntime& runtime,
    const std::filesystem::path& export_root,
    const tracer_core::core::dto::ReportExportRequest& request);

}  // namespace tracer_core::core::c_api::reporting

#endif  // APPS_TRACER_CORE_SHELL_API_C_API_CAPABILITIES_REPORTING_TRACER_CORE_C_API_REPORTING_INTERNAL_HPP_
