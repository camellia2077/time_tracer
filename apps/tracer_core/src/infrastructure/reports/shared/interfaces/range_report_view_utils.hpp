// infrastructure/reports/shared/interfaces/range_report_view_utils.hpp
#ifndef INFRASTRUCTURE_REPORTS_SHARED_INTERFACES_RANGE_REPORT_VIEW_UTILS_H_
#define INFRASTRUCTURE_REPORTS_SHARED_INTERFACES_RANGE_REPORT_VIEW_UTILS_H_

#include <cstddef>
#include <string>
#include <type_traits>

#include "domain/reports/models/range_report_data.hpp"
#include "infrastructure/reports/shared/interfaces/formatter_c_abi_v2.hpp"

namespace range_report_view_utils {

inline auto ToOwnedString(const TtStringView& view) -> std::string {
  if ((view.data == nullptr) || (view.length == 0U)) {
    return {};
  }
  return {view.data, static_cast<size_t>(view.length)};
}

template <typename RangeDataType>
inline auto BuildRangeLikeSummaryData(const TtRangeReportDataV1& data_view)
    -> RangeDataType {
  static_assert(std::is_base_of_v<RangeReportData, RangeDataType>,
                "RangeDataType must inherit RangeReportData.");

  RangeDataType data{};
  data.range_label = ToOwnedString(data_view.rangeLabel);
  data.start_date = ToOwnedString(data_view.startDate);
  data.end_date = ToOwnedString(data_view.endDate);
  data.requested_days = data_view.requestedDays;
  data.total_duration = data_view.totalDuration;
  data.actual_days = data_view.actualDays;
  data.status_true_days = data_view.statusTrueDays;
  data.sleep_true_days = data_view.sleepTrueDays;
  data.exercise_true_days = data_view.exerciseTrueDays;
  data.cardio_true_days = data_view.cardioTrueDays;
  data.anaerobic_true_days = data_view.anaerobicTrueDays;
  data.is_valid = (data_view.isValid != 0U);
  return data;
}

}  // namespace range_report_view_utils

#endif  // INFRASTRUCTURE_REPORTS_SHARED_INTERFACES_RANGE_REPORT_VIEW_UTILS_H_
