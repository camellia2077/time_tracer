// infrastructure/reports/daily/formatters/common/day_report_view_utils.hpp
#ifndef REPORTS_DAILY_FORMATTERS_COMMON_DAY_REPORT_VIEW_UTILS_H_
#define REPORTS_DAILY_FORMATTERS_COMMON_DAY_REPORT_VIEW_UTILS_H_

#include <cstddef>
#include <string>
#include <utility>

#include "domain/reports/models/daily_report_data.hpp"
#include "infrastructure/reports/shared/interfaces/formatter_c_abi_v2.hpp"

namespace day_report_view_utils {

inline auto ToOwnedString(const TtStringView& view) -> std::string {
  if ((view.data == nullptr) || (view.length == 0U)) {
    return {};
  }
  return {view.data, static_cast<size_t>(view.length)};
}

inline auto BuildDailyContentData(const TtDailyReportDataV1& data_view)
    -> DailyReportData {
  DailyReportData data{};
  data.date = ToOwnedString(data_view.date);
  data.metadata.status = ToOwnedString(data_view.metadata.status);
  data.metadata.sleep = ToOwnedString(data_view.metadata.sleep);
  data.metadata.remark = ToOwnedString(data_view.metadata.remark);
  data.metadata.getup_time = ToOwnedString(data_view.metadata.getupTime);
  data.metadata.exercise = ToOwnedString(data_view.metadata.exercise);
  data.total_duration = data_view.totalDuration;

  data.detailed_records.reserve(data_view.detailedRecordCount);
  for (uint32_t record_index = 0; record_index < data_view.detailedRecordCount;
       ++record_index) {
    const auto& record_view = data_view.detailedRecords[record_index];
    TimeRecord record{};
    record.start_time = ToOwnedString(record_view.startTime);
    record.end_time = ToOwnedString(record_view.endTime);
    record.project_path = ToOwnedString(record_view.projectPath);
    record.duration_seconds = record_view.durationSeconds;
    if (record_view.hasActivityRemark != 0U) {
      record.activityRemark = ToOwnedString(record_view.activityRemark);
    }
    data.detailed_records.push_back(std::move(record));
  }

  for (uint32_t stat_index = 0; stat_index < data_view.statsCount;
       ++stat_index) {
    const auto& stat_view = data_view.stats[stat_index];
    data.stats[ToOwnedString(stat_view.key)] = stat_view.value;
  }

  return data;
}

}  // namespace day_report_view_utils

#endif  // REPORTS_DAILY_FORMATTERS_COMMON_DAY_REPORT_VIEW_UTILS_H_
