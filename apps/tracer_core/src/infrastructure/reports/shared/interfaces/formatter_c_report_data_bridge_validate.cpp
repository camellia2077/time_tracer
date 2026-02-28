// infrastructure/reports/shared/interfaces/formatter_c_report_data_bridge_validate.cpp
#include <string>
#include <utility>

#include "infrastructure/reports/shared/interfaces/formatter_c_report_data_bridge.hpp"
#include "infrastructure/reports/shared/interfaces/formatter_c_report_data_bridge_internal.hpp"

namespace formatter_c_report_data_bridge {

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
auto ValidateDailyReportDataView(const void* report_data,
                                 const TtDailyReportDataV1** out_data,
                                 std::string* error_message) -> bool {
  if ((out_data == nullptr) || (error_message == nullptr)) {
    return false;
  }
  *out_data = nullptr;
  error_message->clear();

  if (report_data == nullptr) {
    *error_message = "report_data must not be null.";
    return false;
  }

  const auto* daily_view = static_cast<const TtDailyReportDataV1*>(report_data);
  if ((daily_view->structSize != sizeof(TtDailyReportDataV1)) ||
      (daily_view->version != TT_REPORT_DATA_VERSION_V1)) {
    *error_message = "Invalid daily report data view header.";
    return false;
  }
  if ((daily_view->detailedRecordCount > 0U) &&
      (daily_view->detailedRecords == nullptr)) {
    *error_message = "detailedRecords is null while detailedRecordCount > 0.";
    return false;
  }
  if ((daily_view->statsCount > 0U) && (daily_view->stats == nullptr)) {
    *error_message = "stats is null while statsCount > 0.";
    return false;
  }
  if ((daily_view->projectTreeNodeCount > 0U) &&
      (daily_view->projectTreeNodes == nullptr)) {
    *error_message = "projectTreeNodes is null while projectTreeNodeCount > 0.";
    return false;
  }

  if (!detail::ValidateStringView(daily_view->date, "daily.date", true,
                                  error_message)) {
    return false;
  }
  if (!detail::ValidateStringView(daily_view->metadata.status,
                                  "daily.metadata.status", true,
                                  error_message)) {
    return false;
  }
  if (!detail::ValidateStringView(daily_view->metadata.sleep,
                                  "daily.metadata.sleep", true,
                                  error_message)) {
    return false;
  }
  if (!detail::ValidateStringView(daily_view->metadata.remark,
                                  "daily.metadata.remark", true,
                                  error_message)) {
    return false;
  }
  if (!detail::ValidateStringView(daily_view->metadata.getupTime,
                                  "daily.metadata.getupTime", true,
                                  error_message)) {
    return false;
  }
  if (!detail::ValidateStringView(daily_view->metadata.exercise,
                                  "daily.metadata.exercise", true,
                                  error_message)) {
    return false;
  }

  for (uint32_t record_index = 0;
       record_index < daily_view->detailedRecordCount; ++record_index) {
    const auto& record_view = daily_view->detailedRecords[record_index];
    if (!detail::ValidateStringView(record_view.startTime,
                                    "daily.record.startTime", true,
                                    error_message)) {
      return false;
    }
    if (!detail::ValidateStringView(record_view.endTime, "daily.record.endTime",
                                    true, error_message)) {
      return false;
    }
    if (!detail::ValidateStringView(record_view.projectPath,
                                    "daily.record.projectPath", true,
                                    error_message)) {
      return false;
    }
    if ((record_view.hasActivityRemark != 0U) &&
        !detail::ValidateStringView(record_view.activityRemark,
                                    "daily.record.activityRemark", true,
                                    error_message)) {
      return false;
    }
  }

  for (uint32_t stat_index = 0; stat_index < daily_view->statsCount;
       ++stat_index) {
    if (!detail::ValidateStringView(daily_view->stats[stat_index].key,
                                    "daily.stats.key", false, error_message)) {
      return false;
    }
  }

  for (uint32_t node_index = 0; node_index < daily_view->projectTreeNodeCount;
       ++node_index) {
    const auto& node_view = daily_view->projectTreeNodes[node_index];
    if (!detail::ValidateStringView(node_view.name, "projectTreeNodes.name",
                                    false, error_message)) {
      return false;
    }
    if (node_view.parentIndex < -1) {
      *error_message = "Invalid projectTreeNodes.parentIndex.";
      return false;
    }
    if (std::cmp_greater_equal(node_view.parentIndex,
                               daily_view->projectTreeNodeCount) ||
        std::cmp_greater_equal(node_view.parentIndex, node_index)) {
      *error_message = "Invalid projectTreeNodes.parentIndex.";
      return false;
    }
  }

  *out_data = daily_view;
  return true;
}

auto ValidateRangeReportDataView(const void* report_data,
                                 const TtRangeReportDataV1** out_data,
                                 std::string* error_message) -> bool {
  if ((out_data == nullptr) || (error_message == nullptr)) {
    return false;
  }
  *out_data = nullptr;
  error_message->clear();

  if (report_data == nullptr) {
    *error_message = "report_data must not be null.";
    return false;
  }

  const auto* range_view = static_cast<const TtRangeReportDataV1*>(report_data);
  if ((range_view->structSize != sizeof(TtRangeReportDataV1)) ||
      (range_view->version != TT_REPORT_DATA_VERSION_V1)) {
    *error_message = "Invalid range report data view header.";
    return false;
  }
  if ((range_view->projectTreeNodeCount > 0U) &&
      (range_view->projectTreeNodes == nullptr)) {
    *error_message = "projectTreeNodes is null while projectTreeNodeCount > 0.";
    return false;
  }
  if (!detail::ValidateStringView(range_view->rangeLabel, "range.rangeLabel",
                                  true, error_message)) {
    return false;
  }
  if (!detail::ValidateStringView(range_view->startDate, "range.startDate",
                                  true, error_message)) {
    return false;
  }
  if (!detail::ValidateStringView(range_view->endDate, "range.endDate", true,
                                  error_message)) {
    return false;
  }

  for (uint32_t node_index = 0; node_index < range_view->projectTreeNodeCount;
       ++node_index) {
    const auto& node_view = range_view->projectTreeNodes[node_index];
    if (!detail::ValidateStringView(node_view.name, "projectTreeNodes.name",
                                    false, error_message)) {
      return false;
    }

    if (node_view.parentIndex < -1) {
      *error_message = "Invalid projectTreeNodes.parentIndex.";
      return false;
    }
    if (std::cmp_greater_equal(node_view.parentIndex,
                               range_view->projectTreeNodeCount) ||
        std::cmp_greater_equal(node_view.parentIndex, node_index)) {
      *error_message = "Invalid projectTreeNodes.parentIndex.";
      return false;
    }
  }

  *out_data = range_view;
  return true;
}

}  // namespace formatter_c_report_data_bridge
