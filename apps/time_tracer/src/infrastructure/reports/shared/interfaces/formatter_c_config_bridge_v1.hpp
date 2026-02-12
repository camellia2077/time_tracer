// infrastructure/reports/shared/interfaces/formatter_c_config_bridge_v1.hpp
#ifndef REPORTS_SHARED_INTERFACES_FORMATTER_C_CONFIG_BRIDGE_V1_H_
#define REPORTS_SHARED_INTERFACES_FORMATTER_C_CONFIG_BRIDGE_V1_H_

#include <cstddef>
#include <cstdint>
#include <string>

#include "infrastructure/reports/shared/api/shared_api.hpp"
#include "infrastructure/reports/shared/interfaces/formatter_c_abi_v2.hpp"

namespace formatter_c_config_bridge_v1 {

REPORTS_SHARED_API auto GetDayMdConfigView(
    const TtFormatterConfig* formatter_config,
    const TtDayMdConfigV1** out_config, std::string* error_message) -> bool;

REPORTS_SHARED_API auto GetDayTexConfigView(
    const TtFormatterConfig* formatter_config,
    const TtDayTexConfigV1** out_config, std::string* error_message) -> bool;

REPORTS_SHARED_API auto GetDayTypConfigView(
    const TtFormatterConfig* formatter_config,
    const TtDayTypConfigV1** out_config, std::string* error_message) -> bool;

REPORTS_SHARED_API auto GetMonthMdConfigView(
    const TtFormatterConfig* formatter_config,
    const TtMonthMdConfigV1** out_config, std::string* error_message) -> bool;

REPORTS_SHARED_API auto GetMonthTexConfigView(
    const TtFormatterConfig* formatter_config,
    const TtMonthTexConfigV1** out_config, std::string* error_message) -> bool;

REPORTS_SHARED_API auto GetMonthTypConfigView(
    const TtFormatterConfig* formatter_config,
    const TtMonthTypConfigV1** out_config, std::string* error_message) -> bool;

REPORTS_SHARED_API auto GetRangeMdConfigView(
    const TtFormatterConfig* formatter_config,
    const TtRangeMdConfigV1** out_config, std::string* error_message) -> bool;

REPORTS_SHARED_API auto GetRangeTexConfigView(
    const TtFormatterConfig* formatter_config,
    const TtRangeTexConfigV1** out_config, std::string* error_message) -> bool;

REPORTS_SHARED_API auto GetRangeTypConfigView(
    const TtFormatterConfig* formatter_config,
    const TtRangeTypConfigV1** out_config, std::string* error_message) -> bool;

REPORTS_SHARED_API auto ValidateReportDataView(
    const void* report_data, uint32_t report_data_kind,
    const uint32_t* supported_kinds, size_t supported_kind_count,
    const char* formatter_name, const char* expected_kinds_hint,
    const void** out_native_data, int32_t* out_error_code,
    std::string* out_error_message) -> bool;

}  // namespace formatter_c_config_bridge_v1

#endif  // REPORTS_SHARED_INTERFACES_FORMATTER_C_CONFIG_BRIDGE_V1_H_
