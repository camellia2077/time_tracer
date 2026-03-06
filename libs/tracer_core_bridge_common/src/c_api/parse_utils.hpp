#ifndef TRACER_CORE_BRIDGE_COMMON_C_API_PARSE_UTILS_HPP_
#define TRACER_CORE_BRIDGE_COMMON_C_API_PARSE_UTILS_HPP_

#include <string>

#include "application/dto/core_requests.hpp"
#include "application/dto/core_responses.hpp"
#include "domain/reports/types/report_types.hpp"
#include "domain/types/date_check_mode.hpp"

namespace tracer_core_bridge_common::c_api {

[[nodiscard]] auto ToLowerAscii(std::string value) -> std::string;

[[nodiscard]] auto ParseDateCheckMode(const std::string& value)
    -> DateCheckMode;
[[nodiscard]] auto ParseIngestMode(const std::string& value) -> IngestMode;
[[nodiscard]] auto ParseQueryAction(const std::string& value)
    -> tracer_core::core::dto::DataQueryAction;
[[nodiscard]] auto ParseDataQueryOutputMode(const std::string& value)
    -> tracer_core::core::dto::DataQueryOutputMode;
[[nodiscard]] auto ParseExportType(const std::string& value)
    -> tracer_core::core::dto::ReportExportType;
[[nodiscard]] auto ParseReportType(const std::string& value)
    -> tracer_core::core::dto::ReportQueryType;
[[nodiscard]] auto ParseReportFormat(const std::string& value) -> ReportFormat;

}  // namespace tracer_core_bridge_common::c_api

#endif  // TRACER_CORE_BRIDGE_COMMON_C_API_PARSE_UTILS_HPP_
