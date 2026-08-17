#ifndef API_CORE_C_API_PARSE_BRIDGE_H_
#define API_CORE_C_API_PARSE_BRIDGE_H_

#include <string>

#include "domain/types/date_check_mode.hpp"
#include "domain/types/time_order_mode.hpp"

enum class IngestMode;
enum class InsightsFormat;

namespace tracer_core::core::dto {

enum class DataQueryAction;
enum class DataQueryOutputMode;
enum class InsightsAverageDayBasis;
enum class InsightsDisplayMode;
enum class InsightsExportScope;
enum class InsightsOperationKind;
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
[[nodiscard]] auto ParseInsightsAverageDayBasis(const std::string& value)
    -> tracer_core::core::dto::InsightsAverageDayBasis;
[[nodiscard]] auto ParseInsightsDisplayMode(const std::string& value)
    -> tracer_core::core::dto::InsightsDisplayMode;
[[nodiscard]] auto ParseInsightsExportScope(const std::string& value)
    -> tracer_core::core::dto::InsightsExportScope;
[[nodiscard]] auto ParseInsightsOperationKind(const std::string& value)
    -> tracer_core::core::dto::InsightsOperationKind;
[[nodiscard]] auto ParseTemporalSelectionKind(const std::string& value)
    -> tracer_core::core::dto::TemporalSelectionKind;
[[nodiscard]] auto ParseInsightsFormat(const std::string& value)
    -> InsightsFormat;

}  // namespace tracer_core::shell::c_api_bridge

#endif  // API_CORE_C_API_PARSE_BRIDGE_H_
