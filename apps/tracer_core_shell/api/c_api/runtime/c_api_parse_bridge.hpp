#ifndef API_CORE_C_API_PARSE_BRIDGE_H_
#define API_CORE_C_API_PARSE_BRIDGE_H_

#include <string>

#include "domain/types/date_check_mode.hpp"

enum class IngestMode;
enum class ReportFormat;

namespace tracer_core::core::dto {

enum class DataQueryAction;
enum class DataQueryOutputMode;
enum class ReportExportType;
enum class ReportQueryType;
enum class ReportTargetType;

}  // namespace tracer_core::core::dto

namespace tracer_core::shell::c_api_bridge {

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
[[nodiscard]] auto ParseReportTargetType(const std::string& value)
    -> tracer_core::core::dto::ReportTargetType;
[[nodiscard]] auto ParseReportFormat(const std::string& value) -> ReportFormat;

}  // namespace tracer_core::shell::c_api_bridge

#endif  // API_CORE_C_API_PARSE_BRIDGE_H_
