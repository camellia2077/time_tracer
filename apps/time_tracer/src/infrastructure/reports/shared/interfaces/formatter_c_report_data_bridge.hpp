// infrastructure/reports/shared/interfaces/formatter_c_report_data_bridge.hpp
#ifndef REPORTS_SHARED_INTERFACES_FORMATTER_C_REPORT_DATA_BRIDGE_H_
#define REPORTS_SHARED_INTERFACES_FORMATTER_C_REPORT_DATA_BRIDGE_H_

#include <string>

#include "domain/reports/models/daily_report_data.hpp"
#include "domain/reports/models/range_report_data.hpp"
#include "infrastructure/reports/shared/api/shared_api.hpp"
#include "infrastructure/reports/shared/interfaces/formatter_c_abi_v2.hpp"

namespace formatter_c_report_data_bridge {

REPORTS_SHARED_API auto ValidateDailyReportDataView(
    const void* report_data, const TtDailyReportDataV1** out_data,
    std::string* error_message) -> bool;

REPORTS_SHARED_API auto ValidateRangeReportDataView(
    const void* report_data, const TtRangeReportDataV1** out_data,
    std::string* error_message) -> bool;

REPORTS_SHARED_API auto BuildDailyReportDataFromView(const void* report_data,
                                                     DailyReportData* out_data,
                                                     std::string* error_message)
    -> bool;

REPORTS_SHARED_API auto BuildRangeReportDataFromView(const void* report_data,
                                                     RangeReportData* out_data,
                                                     std::string* error_message)
    -> bool;

}  // namespace formatter_c_report_data_bridge

#endif  // REPORTS_SHARED_INTERFACES_FORMATTER_C_REPORT_DATA_BRIDGE_H_
