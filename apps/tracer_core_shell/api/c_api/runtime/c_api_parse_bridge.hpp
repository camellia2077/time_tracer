#ifndef API_CORE_C_API_PARSE_BRIDGE_H_
#define API_CORE_C_API_PARSE_BRIDGE_H_

#include <string>

#include "domain/types/date_check_mode.hpp"
#include "domain/types/time_order_mode.hpp"

enum class IngestMode;
enum class ReportFormat;

namespace tracer_core::core::dto {

enum class DataQueryAction;
enum class DataQueryOutputMode;
enum class ReportDisplayMode;
enum class ReportExportScope;
enum class ReportOperationKind;
enum class TemporalSelectionKind;

}  // namespace tracer_core::core::dto

namespace tracer_core::shell::c_api_bridge {

[[nodiscard]] auto ToLowerAscii(std::string value) -> std::string;

[[nodiscard]] auto ParseDateCheckMode(const std::string& value)
    -> DateCheckMode;
[[nodiscard]] auto ParseIngestMode(const std::string& value) -> IngestMode;
[[nodiscard]] auto ParseTimeOrderMode(const std::string& value)
    -> TimeOrderMode;
[[nodiscard]] auto ParseQueryAction(const std::string& value)
    -> tracer_core::core::dto::DataQueryAction;
[[nodiscard]] auto ParseDataQueryOutputMode(const std::string& value)
    -> tracer_core::core::dto::DataQueryOutputMode;
[[nodiscard]] auto ParseReportDisplayMode(const std::string& value)
    -> tracer_core::core::dto::ReportDisplayMode;
[[nodiscard]] auto ParseReportExportScope(const std::string& value)
    -> tracer_core::core::dto::ReportExportScope;
[[nodiscard]] auto ParseReportOperationKind(const std::string& value)
    -> tracer_core::core::dto::ReportOperationKind;
[[nodiscard]] auto ParseTemporalSelectionKind(const std::string& value)
    -> tracer_core::core::dto::TemporalSelectionKind;
[[nodiscard]] auto ParseReportFormat(const std::string& value) -> ReportFormat;

}  // namespace tracer_core::shell::c_api_bridge

#endif  // API_CORE_C_API_PARSE_BRIDGE_H_
