// infrastructure/reports/shared/interfaces/formatter_c_report_data_bridge_parse.cpp
#include <string>
#include <utility>
#include <vector>

#include "infrastructure/reports/shared/interfaces/formatter_c_report_data_bridge.hpp"
#include "infrastructure/reports/shared/interfaces/formatter_c_report_data_bridge_internal.hpp"

namespace formatter_c_report_data_bridge {
namespace {

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
    if (!detail::ParseStringView(node_view.name, "projectTreeNodes.name",
                                 &node_name, error_message)) {
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

auto ParseDailyMetadata(const TtDailyReportDataV1& daily_view,
                        DailyReportData* parsed_data,
                        std::string* error_message) -> bool {
  if ((parsed_data == nullptr) || (error_message == nullptr)) {
    return false;
  }
  if (!detail::ParseStringView(daily_view.date, "daily.date",
                               &parsed_data->date, error_message)) {
    return false;
  }
  if (!detail::ParseStringView(daily_view.metadata.status,
                               "daily.metadata.status",
                               &parsed_data->metadata.status, error_message)) {
    return false;
  }
  if (!detail::ParseStringView(daily_view.metadata.sleep,
                               "daily.metadata.sleep",
                               &parsed_data->metadata.sleep, error_message)) {
    return false;
  }
  if (!detail::ParseStringView(daily_view.metadata.remark,
                               "daily.metadata.remark",
                               &parsed_data->metadata.remark, error_message)) {
    return false;
  }
  if (!detail::ParseStringView(
          daily_view.metadata.getupTime, "daily.metadata.getupTime",
          &parsed_data->metadata.getup_time, error_message)) {
    return false;
  }
  if (!detail::ParseStringView(
          daily_view.metadata.exercise, "daily.metadata.exercise",
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
    if (!detail::ParseStringView(record_view.startTime,
                                 "daily.record.startTime",
                                 &parsed_record.start_time, error_message)) {
      return false;
    }
    if (!detail::ParseStringView(record_view.endTime, "daily.record.endTime",
                                 &parsed_record.end_time, error_message)) {
      return false;
    }
    if (!detail::ParseStringView(record_view.projectPath,
                                 "daily.record.projectPath",
                                 &parsed_record.project_path, error_message)) {
      return false;
    }
    parsed_record.duration_seconds = record_view.durationSeconds;

    if (record_view.hasActivityRemark != 0U) {
      std::string activity_remark;
      if (!detail::ParseStringView(record_view.activityRemark,
                                   "daily.record.activityRemark",
                                   &activity_remark, error_message)) {
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
    if (!detail::ParseStringView(stat_view.key, "daily.stats.key", &stat_key,
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

}  // namespace

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
  if (!detail::ParseStringView(range_view->rangeLabel, "range.rangeLabel",
                               &parsed_data.range_label, error_message)) {
    return false;
  }
  if (!detail::ParseStringView(range_view->startDate, "range.startDate",
                               &parsed_data.start_date, error_message)) {
    return false;
  }
  if (!detail::ParseStringView(range_view->endDate, "range.endDate",
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
