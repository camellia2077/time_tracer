#include "infrastructure/reports/shared/interfaces/formatter_c_report_data_bridge.hpp"

#include <limits>
#include <string>
#include <utility>
#include <vector>

namespace formatter_c_report_data_bridge {
namespace {

auto ValidateStringView(const TtStringView& view, const char* field_name,
                        bool allow_empty, std::string* error_message) -> bool {
  if (error_message == nullptr) {
    return false;
  }

  if (view.length == 0U) {
    if (!allow_empty) {
      *error_message =
          std::string("Field '") + field_name + "' must not be empty.";
      return false;
    }
    return true;
  }
  if (view.data == nullptr) {
    *error_message = std::string("Invalid string view for field '") +
                     field_name + "': data is null while length > 0.";
    return false;
  }
  if (view.length >
      static_cast<uint64_t>(std::numeric_limits<std::size_t>::max())) {
    *error_message =
        std::string("String length overflow for field '") + field_name + "'.";
    return false;
  }

  return true;
}

auto ParseStringView(const TtStringView& view, const char* field_name,
                     std::string* out_value, std::string* error_message)
    -> bool {
  if ((out_value == nullptr) || (error_message == nullptr)) {
    return false;
  }

  if (!ValidateStringView(view, field_name, true, error_message)) {
    return false;
  }
  if (view.length == 0U) {
    out_value->clear();
    return true;
  }

  out_value->assign(view.data, static_cast<std::size_t>(view.length));
  return true;
}

auto BuildProjectTreeFromNodes(const TtProjectTreeNodeV1* nodes,
                               uint32_t node_count,
                               reporting::ProjectTree* out_tree,
                               std::string* error_message) -> bool {
  if ((out_tree == nullptr) || (error_message == nullptr)) {
    return false;
  }

  out_tree->clear();
  if (node_count == 0U) {
    return true;
  }
  if (nodes == nullptr) {
    *error_message = "projectTreeNodes is null while projectTreeNodeCount > 0.";
    return false;
  }

  std::vector<reporting::ProjectNode*> node_refs(node_count, nullptr);
  for (uint32_t node_index = 0; node_index < node_count; ++node_index) {
    const TtProjectTreeNodeV1& node_view = nodes[node_index];

    std::string node_name;
    if (!ParseStringView(node_view.name, "projectTreeNodes.name", &node_name,
                         error_message)) {
      return false;
    }
    if (node_name.empty()) {
      *error_message = "projectTreeNodes.name must not be empty.";
      return false;
    }

    reporting::ProjectNode* target_node = nullptr;
    if (node_view.parentIndex < 0) {
      target_node = &(*out_tree)[node_name];
    } else {
      const int32_t kParentIndex = node_view.parentIndex;
      if (std::cmp_greater_equal(kParentIndex, node_count) ||
          std::cmp_greater_equal(kParentIndex, node_index)) {
        *error_message = "Invalid projectTreeNodes.parentIndex.";
        return false;
      }

      reporting::ProjectNode* parent_node = node_refs[kParentIndex];
      if (parent_node == nullptr) {
        *error_message =
            "Invalid projectTreeNodes ordering: parent node unavailable.";
        return false;
      }
      target_node = &parent_node->children[node_name];
    }

    target_node->duration = node_view.duration;
    node_refs[node_index] = target_node;
  }

  return true;
}

}  // namespace

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

  if (!ValidateStringView(daily_view->date, "daily.date", true,
                          error_message)) {
    return false;
  }
  if (!ValidateStringView(daily_view->metadata.status, "daily.metadata.status",
                          true, error_message)) {
    return false;
  }
  if (!ValidateStringView(daily_view->metadata.sleep, "daily.metadata.sleep",
                          true, error_message)) {
    return false;
  }
  if (!ValidateStringView(daily_view->metadata.remark, "daily.metadata.remark",
                          true, error_message)) {
    return false;
  }
  if (!ValidateStringView(daily_view->metadata.getupTime,
                          "daily.metadata.getupTime", true, error_message)) {
    return false;
  }
  if (!ValidateStringView(daily_view->metadata.exercise,
                          "daily.metadata.exercise", true, error_message)) {
    return false;
  }

  for (uint32_t record_index = 0;
       record_index < daily_view->detailedRecordCount; ++record_index) {
    const auto& record_view = daily_view->detailedRecords[record_index];
    if (!ValidateStringView(record_view.startTime, "daily.record.startTime",
                            true, error_message)) {
      return false;
    }
    if (!ValidateStringView(record_view.endTime, "daily.record.endTime", true,
                            error_message)) {
      return false;
    }
    if (!ValidateStringView(record_view.projectPath, "daily.record.projectPath",
                            true, error_message)) {
      return false;
    }
    if ((record_view.hasActivityRemark != 0U) &&
        !ValidateStringView(record_view.activityRemark,
                            "daily.record.activityRemark", true,
                            error_message)) {
      return false;
    }
  }

  for (uint32_t stat_index = 0; stat_index < daily_view->statsCount;
       ++stat_index) {
    if (!ValidateStringView(daily_view->stats[stat_index].key,
                            "daily.stats.key", false, error_message)) {
      return false;
    }
  }

  for (uint32_t node_index = 0; node_index < daily_view->projectTreeNodeCount;
       ++node_index) {
    const auto& node_view = daily_view->projectTreeNodes[node_index];
    if (!ValidateStringView(node_view.name, "projectTreeNodes.name", false,
                            error_message)) {
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
  if (!ValidateStringView(range_view->rangeLabel, "range.rangeLabel", true,
                          error_message)) {
    return false;
  }
  if (!ValidateStringView(range_view->startDate, "range.startDate", true,
                          error_message)) {
    return false;
  }
  if (!ValidateStringView(range_view->endDate, "range.endDate", true,
                          error_message)) {
    return false;
  }

  for (uint32_t node_index = 0; node_index < range_view->projectTreeNodeCount;
       ++node_index) {
    const auto& node_view = range_view->projectTreeNodes[node_index];
    if (!ValidateStringView(node_view.name, "projectTreeNodes.name", false,
                            error_message)) {
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

auto ParseDailyMetadata(const TtDailyReportDataV1& daily_view,
                        DailyReportData* parsed_data,
                        std::string* error_message) -> bool {
  if ((parsed_data == nullptr) || (error_message == nullptr)) {
    return false;
  }
  if (!ParseStringView(daily_view.date, "daily.date", &parsed_data->date,
                       error_message)) {
    return false;
  }
  if (!ParseStringView(daily_view.metadata.status, "daily.metadata.status",
                       &parsed_data->metadata.status, error_message)) {
    return false;
  }
  if (!ParseStringView(daily_view.metadata.sleep, "daily.metadata.sleep",
                       &parsed_data->metadata.sleep, error_message)) {
    return false;
  }
  if (!ParseStringView(daily_view.metadata.remark, "daily.metadata.remark",
                       &parsed_data->metadata.remark, error_message)) {
    return false;
  }
  if (!ParseStringView(daily_view.metadata.getupTime,
                       "daily.metadata.getupTime",
                       &parsed_data->metadata.getup_time, error_message)) {
    return false;
  }
  if (!ParseStringView(daily_view.metadata.exercise, "daily.metadata.exercise",
                       &parsed_data->metadata.exercise, error_message)) {
    return false;
  }
  parsed_data->total_duration = daily_view.totalDuration;
  return true;
}

auto ParseDailyDetailedRecords(const TtDailyReportDataV1& daily_view,
                               DailyReportData* parsed_data,
                               std::string* error_message) -> bool {
  if ((parsed_data == nullptr) || (error_message == nullptr)) {
    return false;
  }

  parsed_data->detailed_records.reserve(daily_view.detailedRecordCount);
  for (uint32_t record_index = 0; record_index < daily_view.detailedRecordCount;
       ++record_index) {
    const TtDailyTimeRecordV1& record_view =
        daily_view.detailedRecords[record_index];

    TimeRecord parsed_record{};
    if (!ParseStringView(record_view.startTime, "daily.record.startTime",
                         &parsed_record.start_time, error_message)) {
      return false;
    }
    if (!ParseStringView(record_view.endTime, "daily.record.endTime",
                         &parsed_record.end_time, error_message)) {
      return false;
    }
    if (!ParseStringView(record_view.projectPath, "daily.record.projectPath",
                         &parsed_record.project_path, error_message)) {
      return false;
    }
    parsed_record.duration_seconds = record_view.durationSeconds;

    if (record_view.hasActivityRemark != 0U) {
      std::string activity_remark;
      if (!ParseStringView(record_view.activityRemark,
                           "daily.record.activityRemark", &activity_remark,
                           error_message)) {
        return false;
      }
      parsed_record.activityRemark = std::move(activity_remark);
    }

    parsed_data->detailed_records.push_back(std::move(parsed_record));
  }
  return true;
}

auto ParseDailyStats(const TtDailyReportDataV1& daily_view,
                     DailyReportData* parsed_data, std::string* error_message)
    -> bool {
  if ((parsed_data == nullptr) || (error_message == nullptr)) {
    return false;
  }

  for (uint32_t stat_index = 0; stat_index < daily_view.statsCount;
       ++stat_index) {
    const TtStringInt64PairV1& stat_view = daily_view.stats[stat_index];
    std::string stat_key;
    if (!ParseStringView(stat_view.key, "daily.stats.key", &stat_key,
                         error_message)) {
      return false;
    }
    if (stat_key.empty()) {
      *error_message = "daily.stats.key must not be empty.";
      return false;
    }
    parsed_data->stats[stat_key] = stat_view.value;
  }
  return true;
}

auto BuildDailyReportDataFromView(const void* report_data,
                                  DailyReportData* out_data,
                                  std::string* error_message) -> bool {
  if ((out_data == nullptr) || (error_message == nullptr)) {
    return false;
  }
  const TtDailyReportDataV1* daily_view = nullptr;
  if (!ValidateDailyReportDataView(report_data, &daily_view, error_message)) {
    return false;
  }

  DailyReportData parsed_data{};
  if (!ParseDailyMetadata(*daily_view, &parsed_data, error_message)) {
    return false;
  }
  if (!ParseDailyDetailedRecords(*daily_view, &parsed_data, error_message)) {
    return false;
  }
  if (!ParseDailyStats(*daily_view, &parsed_data, error_message)) {
    return false;
  }

  if (!BuildProjectTreeFromNodes(daily_view->projectTreeNodes,
                                 daily_view->projectTreeNodeCount,
                                 &parsed_data.project_tree, error_message)) {
    return false;
  }

  *out_data = std::move(parsed_data);
  return true;
}

auto BuildRangeReportDataFromView(const void* report_data,
                                  RangeReportData* out_data,
                                  std::string* error_message) -> bool {
  if ((out_data == nullptr) || (error_message == nullptr)) {
    return false;
  }
  const TtRangeReportDataV1* range_view = nullptr;
  if (!ValidateRangeReportDataView(report_data, &range_view, error_message)) {
    return false;
  }

  RangeReportData parsed_data{};
  if (!ParseStringView(range_view->rangeLabel, "range.rangeLabel",
                       &parsed_data.range_label, error_message)) {
    return false;
  }
  if (!ParseStringView(range_view->startDate, "range.startDate",
                       &parsed_data.start_date, error_message)) {
    return false;
  }
  if (!ParseStringView(range_view->endDate, "range.endDate",
                       &parsed_data.end_date, error_message)) {
    return false;
  }

  parsed_data.requested_days = range_view->requestedDays;
  parsed_data.total_duration = range_view->totalDuration;
  parsed_data.actual_days = range_view->actualDays;
  parsed_data.status_true_days = range_view->statusTrueDays;
  parsed_data.sleep_true_days = range_view->sleepTrueDays;
  parsed_data.exercise_true_days = range_view->exerciseTrueDays;
  parsed_data.cardio_true_days = range_view->cardioTrueDays;
  parsed_data.anaerobic_true_days = range_view->anaerobicTrueDays;
  parsed_data.is_valid = (range_view->isValid != 0U);

  if (!BuildProjectTreeFromNodes(range_view->projectTreeNodes,
                                 range_view->projectTreeNodeCount,
                                 &parsed_data.project_tree, error_message)) {
    return false;
  }

  *out_data = std::move(parsed_data);
  return true;
}

}  // namespace formatter_c_report_data_bridge
