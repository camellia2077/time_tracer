#ifndef API_CORE_C_TIME_TRACER_CORE_C_API_INTERNAL_H_
#define API_CORE_C_TIME_TRACER_CORE_C_API_INTERNAL_H_

#include <string>
#include <string_view>

#include "api/android/android_runtime_factory.hpp"
#include "application/dto/core_requests.hpp"
#include "application/dto/core_responses.hpp"
#include "application/use_cases/i_time_tracer_core_api.hpp"
#include "domain/reports/types/report_types.hpp"
#include "domain/types/date_check_mode.hpp"

struct TtCoreRuntimeHandle {
  infrastructure::bootstrap::AndroidRuntime runtime;
};

namespace time_tracer::core::c_api::internal {

extern thread_local std::string g_last_error;
extern thread_local std::string g_last_response;

void ClearLastError();
void SetLastError(const char* message);

[[nodiscard]] auto BuildFailureResponse(std::string message) -> const char*;
[[nodiscard]] auto BuildOperationResponse(
    const time_tracer::core::dto::OperationAck& output) -> const char*;
[[nodiscard]] auto BuildTextResponse(
    const time_tracer::core::dto::TextOutput& output) -> const char*;
[[nodiscard]] auto BuildTreeResponse(
    const time_tracer::core::dto::TreeQueryResponse& response) -> const char*;
[[nodiscard]] auto BuildCapabilitiesResponseJson() -> const char*;

[[nodiscard]] auto RequireRuntime(TtCoreRuntimeHandle* handle)
    -> ITimeTracerCoreApi&;

[[nodiscard]] auto ToRequestJsonView(const char* request_json)
    -> std::string_view;

[[nodiscard]] auto ParseDateCheckMode(const std::string& value)
    -> DateCheckMode;
[[nodiscard]] auto ParseIngestMode(const std::string& value) -> IngestMode;
[[nodiscard]] auto ParseQueryAction(const std::string& value)
    -> time_tracer::core::dto::DataQueryAction;
[[nodiscard]] auto ParseDataQueryOutputMode(const std::string& value)
    -> time_tracer::core::dto::DataQueryOutputMode;
[[nodiscard]] auto ParseExportType(const std::string& value)
    -> time_tracer::core::dto::ReportExportType;
[[nodiscard]] auto ParseReportType(const std::string& value)
    -> time_tracer::core::dto::ReportQueryType;
[[nodiscard]] auto ParseReportFormat(const std::string& value) -> ReportFormat;

}  // namespace time_tracer::core::c_api::internal

#endif  // API_CORE_C_TIME_TRACER_CORE_C_API_INTERNAL_H_
