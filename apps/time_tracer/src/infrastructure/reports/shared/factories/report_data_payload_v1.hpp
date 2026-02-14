// infrastructure/reports/shared/factories/report_data_payload_v1.hpp
#ifndef INFRASTRUCTURE_REPORTS_SHARED_FACTORIES_REPORT_DATA_PAYLOAD_V1_H_
#define INFRASTRUCTURE_REPORTS_SHARED_FACTORIES_REPORT_DATA_PAYLOAD_V1_H_

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "domain/reports/models/daily_report_data.hpp"
#include "domain/reports/models/range_report_data.hpp"
#include "infrastructure/reports/shared/interfaces/formatter_c_abi_v2.hpp"

namespace report_data_payload_v1 {
namespace detail {

inline auto ToStringView(const std::string& value) -> TtStringView {
  TtStringView view{};
  if (!value.empty()) {
    view.data = value.data();
  }
  view.length = static_cast<uint64_t>(value.size());
  return view;
}

inline auto ToUint32Count(std::size_t value, const char* label) -> uint32_t {
  if (value > static_cast<std::size_t>(std::numeric_limits<uint32_t>::max())) {
    throw std::overflow_error(std::string("Too many elements in ") + label +
                              " for ABI v1 payload.");
  }
  return static_cast<uint32_t>(value);
}

template <typename Type>
struct AlwaysFalse : std::false_type {};

}  // namespace detail

class ReportDataPayloadV1 {
 public:
  template <typename ReportDataType>
  [[nodiscard]] static auto BuildFrom(const ReportDataType& report_data)
      -> ReportDataPayloadV1 {
    if constexpr (std::is_same_v<ReportDataType, DailyReportData>) {
      return ReportDataPayloadV1(report_data);
    } else if constexpr (std::is_base_of_v<RangeReportData, ReportDataType>) {
      return ReportDataPayloadV1(
          static_cast<const RangeReportData&>(report_data));
    } else {
      static_assert(detail::AlwaysFalse<ReportDataType>::value,
                    "Unsupported report data type for ABI v1 payload.");
    }
  }

  [[nodiscard]] auto Data() const -> const void* { return data_ptr_; }
  [[nodiscard]] auto DataSize() const -> uint64_t { return data_size_; }

 private:
  explicit ReportDataPayloadV1(const DailyReportData& report_data) {
    BuildDaily(report_data);
  }

  explicit ReportDataPayloadV1(const RangeReportData& report_data) {
    BuildRange(report_data);
  }

  void ResetStorage() {
    daily_time_records_.clear();
    daily_stats_.clear();
    project_tree_nodes_.clear();
  }

  void BuildDaily(const DailyReportData& report_data) {
    ResetStorage();

    daily_data_.structSize = static_cast<uint32_t>(sizeof(TtDailyReportDataV1));
    daily_data_.version = TT_REPORT_DATA_VERSION_V1;
    daily_data_.date = detail::ToStringView(report_data.date);
    daily_data_.metadata.status =
        detail::ToStringView(report_data.metadata.status);
    daily_data_.metadata.sleep =
        detail::ToStringView(report_data.metadata.sleep);
    daily_data_.metadata.remark =
        detail::ToStringView(report_data.metadata.remark);
    daily_data_.metadata.getupTime =
        detail::ToStringView(report_data.metadata.getup_time);
    daily_data_.metadata.exercise =
        detail::ToStringView(report_data.metadata.exercise);
    daily_data_.totalDuration = report_data.total_duration;

    daily_time_records_.reserve(report_data.detailed_records.size());
    for (const auto& record : report_data.detailed_records) {
      TtDailyTimeRecordV1 payload_record{};
      payload_record.startTime = detail::ToStringView(record.start_time);
      payload_record.endTime = detail::ToStringView(record.end_time);
      payload_record.projectPath = detail::ToStringView(record.project_path);
      payload_record.durationSeconds = record.duration_seconds;
      if (record.activityRemark.has_value()) {
        payload_record.activityRemark =
            detail::ToStringView(record.activityRemark.value());
        payload_record.hasActivityRemark = 1U;
      } else {
        payload_record.activityRemark = TtStringView{};
        payload_record.hasActivityRemark = 0U;
      }
      std::ranges::fill(payload_record.reserved, 0U);
      daily_time_records_.push_back(payload_record);
    }
    daily_data_.detailedRecords =
        daily_time_records_.empty() ? nullptr : daily_time_records_.data();
    daily_data_.detailedRecordCount =
        detail::ToUint32Count(daily_time_records_.size(), "daily_time_records");

    daily_stats_.reserve(report_data.stats.size());
    for (const auto& [key, value] : report_data.stats) {
      TtStringInt64PairV1 payload_stat{};
      payload_stat.key = detail::ToStringView(key);
      payload_stat.value = value;
      daily_stats_.push_back(payload_stat);
    }
    daily_data_.stats = daily_stats_.empty() ? nullptr : daily_stats_.data();
    daily_data_.statsCount =
        detail::ToUint32Count(daily_stats_.size(), "daily_stats");

    FlattenProjectTree(report_data.project_tree, -1);
    daily_data_.projectTreeNodes =
        project_tree_nodes_.empty() ? nullptr : project_tree_nodes_.data();
    daily_data_.projectTreeNodeCount =
        detail::ToUint32Count(project_tree_nodes_.size(), "project_tree_nodes");
    daily_data_.reserved = 0U;

    data_ptr_ = static_cast<const void*>(&daily_data_);
    data_size_ = static_cast<uint64_t>(sizeof(TtDailyReportDataV1));
  }

  void BuildRange(const RangeReportData& report_data) {
    ResetStorage();

    range_data_.structSize = static_cast<uint32_t>(sizeof(TtRangeReportDataV1));
    range_data_.version = TT_REPORT_DATA_VERSION_V1;
    range_data_.rangeLabel = detail::ToStringView(report_data.range_label);
    range_data_.startDate = detail::ToStringView(report_data.start_date);
    range_data_.endDate = detail::ToStringView(report_data.end_date);
    range_data_.requestedDays = report_data.requested_days;
    range_data_.totalDuration = report_data.total_duration;
    range_data_.actualDays = report_data.actual_days;
    range_data_.statusTrueDays = report_data.status_true_days;
    range_data_.sleepTrueDays = report_data.sleep_true_days;
    range_data_.exerciseTrueDays = report_data.exercise_true_days;
    range_data_.cardioTrueDays = report_data.cardio_true_days;
    range_data_.anaerobicTrueDays = report_data.anaerobic_true_days;
    range_data_.isValid = report_data.is_valid ? 1U : 0U;
    std::ranges::fill(range_data_.reserved0, 0U);

    FlattenProjectTree(report_data.project_tree, -1);
    range_data_.projectTreeNodes =
        project_tree_nodes_.empty() ? nullptr : project_tree_nodes_.data();
    range_data_.projectTreeNodeCount =
        detail::ToUint32Count(project_tree_nodes_.size(), "project_tree_nodes");
    range_data_.reserved = 0U;

    data_ptr_ = static_cast<const void*>(&range_data_);
    data_size_ = static_cast<uint64_t>(sizeof(TtRangeReportDataV1));
  }

  void FlattenProjectTree(const reporting::ProjectTree& tree,
                          int32_t parent_index) {
    for (const auto& [node_name, node] : tree) {
      const auto kCurrentIndex =
          static_cast<int32_t>(project_tree_nodes_.size());
      TtProjectTreeNodeV1 payload_node{};
      payload_node.name = detail::ToStringView(node_name);
      payload_node.duration = node.duration;
      payload_node.parentIndex = parent_index;
      payload_node.reserved = 0U;
      project_tree_nodes_.push_back(payload_node);
      FlattenProjectTree(node.children, kCurrentIndex);
    }
  }

  const void* data_ptr_ = nullptr;
  uint64_t data_size_ = 0U;

  TtDailyReportDataV1 daily_data_{};
  TtRangeReportDataV1 range_data_{};

  std::vector<TtDailyTimeRecordV1> daily_time_records_;
  std::vector<TtStringInt64PairV1> daily_stats_;
  std::vector<TtProjectTreeNodeV1> project_tree_nodes_;
};

}  // namespace report_data_payload_v1

#endif  // INFRASTRUCTURE_REPORTS_SHARED_FACTORIES_REPORT_DATA_PAYLOAD_V1_H_
