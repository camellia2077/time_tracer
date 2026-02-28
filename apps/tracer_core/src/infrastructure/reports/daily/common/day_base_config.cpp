// infrastructure/reports/daily/common/day_base_config.cpp
#include "infrastructure/reports/daily/common/day_base_config.hpp"

#include <algorithm>
#include <map>
#include <stdexcept>
#include <utility>
#include <vector>

#include "infrastructure/reports/shared/interfaces/formatter_c_string_view_utils.hpp"

namespace {

auto BuildStatisticItemTree(const std::vector<StatisticItemConfig>& flat_items,
                            const std::vector<std::vector<uint32_t>>& children,
                            uint32_t index) -> StatisticItemConfig {
  StatisticItemConfig item = flat_items[index];
  item.sub_items.clear();
  item.sub_items.reserve(children[index].size());

  for (uint32_t child_index : children[index]) {
    item.sub_items.push_back(
        BuildStatisticItemTree(flat_items, children, child_index));
  }
  return item;
}

}  // namespace

DayBaseConfig::DayBaseConfig(
    const TtDayLabelsConfigV1& labels,
    const TtFormatterStatisticItemNodeV1* statistics_items,
    uint32_t statistics_item_count)
    : statistics_items_(
          BuildStatisticsItems(statistics_items, statistics_item_count)) {
  LoadBaseConfig(labels);
}

auto DayBaseConfig::BuildStatisticsItems(
    const TtFormatterStatisticItemNodeV1* statistics_items,
    uint32_t statistics_item_count) -> std::vector<StatisticItemConfig> {
  if (statistics_item_count == 0U) {
    return {};
  }
  if (statistics_items == nullptr) {
    throw std::invalid_argument(
        "statistics_items is null while statistics_item_count > 0.");
  }

  std::vector<StatisticItemConfig> flat_items;
  flat_items.reserve(statistics_item_count);
  std::vector<std::vector<uint32_t>> children(statistics_item_count);
  std::vector<uint32_t> roots;
  roots.reserve(statistics_item_count);

  for (uint32_t index = 0; index < statistics_item_count; ++index) {
    const auto& source = statistics_items[index];
    StatisticItemConfig item{};
    item.label = formatter_c_string_view_utils::ToString(
        source.label, "statistics_items.label");
    item.db_column = formatter_c_string_view_utils::ToString(
        source.dbColumn, "statistics_items.db_column");
    item.show = source.show != 0U;
    flat_items.push_back(std::move(item));

    const int32_t kParentIndex = source.parentIndex;
    if (kParentIndex < -1) {
      throw std::invalid_argument(
          "statistics_items.parentIndex must be >= -1.");
    }
    if (kParentIndex == -1) {
      roots.push_back(index);
      continue;
    }
    if (std::cmp_greater_equal(kParentIndex, statistics_item_count) ||
        std::cmp_greater_equal(kParentIndex, index)) {
      throw std::invalid_argument("Invalid statistics_items.parentIndex.");
    }
    children[static_cast<uint32_t>(kParentIndex)].push_back(index);
  }

  std::vector<StatisticItemConfig> tree_items;
  tree_items.reserve(roots.size());
  for (uint32_t root_index : roots) {
    tree_items.push_back(
        BuildStatisticItemTree(flat_items, children, root_index));
  }
  return tree_items;
}

void DayBaseConfig::LoadBaseConfig(const TtDayLabelsConfigV1& labels) {
  title_prefix_ = formatter_c_string_view_utils::ToString(labels.titlePrefix,
                                                          "labels.titlePrefix");
  date_label_ = formatter_c_string_view_utils::ToString(labels.dateLabel,
                                                        "labels.dateLabel");
  total_time_label_ = formatter_c_string_view_utils::ToString(
      labels.totalTimeLabel, "labels.totalTimeLabel");
  status_label_ = formatter_c_string_view_utils::ToString(labels.statusLabel,
                                                          "labels.statusLabel");
  sleep_label_ = formatter_c_string_view_utils::ToString(labels.sleepLabel,
                                                         "labels.sleepLabel");
  getup_time_label_ = formatter_c_string_view_utils::ToString(
      labels.getupTimeLabel, "labels.getupTimeLabel");
  remark_label_ = formatter_c_string_view_utils::ToString(labels.remarkLabel,
                                                          "labels.remarkLabel");
  exercise_label_ = formatter_c_string_view_utils::ToString(
      labels.exerciseLabel, "labels.exerciseLabel");
  no_records_ = formatter_c_string_view_utils::ToString(
      labels.noRecordsMessage, "labels.noRecordsMessage");
  statistics_label_ = formatter_c_string_view_utils::ToString(
      labels.statisticsLabel, "labels.statisticsLabel");
  all_activities_label_ = formatter_c_string_view_utils::ToString(
      labels.allActivitiesLabel, "labels.allActivitiesLabel");
  activity_remark_label_ = formatter_c_string_view_utils::ToString(
      labels.activityRemarkLabel, "labels.activityRemarkLabel");
  activity_connector_ = formatter_c_string_view_utils::ToString(
      labels.activityConnector, "labels.activityConnector");
  project_breakdown_label_ = formatter_c_string_view_utils::ToString(
      labels.projectBreakdownLabel, "labels.projectBreakdownLabel");
}

auto DayBaseConfig::GetTitlePrefix() const -> const std::string& {
  return title_prefix_;
}
auto DayBaseConfig::GetDateLabel() const -> const std::string& {
  return date_label_;
}
auto DayBaseConfig::GetTotalTimeLabel() const -> const std::string& {
  return total_time_label_;
}
auto DayBaseConfig::GetStatusLabel() const -> const std::string& {
  return status_label_;
}
auto DayBaseConfig::GetSleepLabel() const -> const std::string& {
  return sleep_label_;
}
auto DayBaseConfig::GetGetupTimeLabel() const -> const std::string& {
  return getup_time_label_;
}
auto DayBaseConfig::GetRemarkLabel() const -> const std::string& {
  return remark_label_;
}
auto DayBaseConfig::GetExerciseLabel() const -> const std::string& {
  return exercise_label_;
}
auto DayBaseConfig::GetNoRecords() const -> const std::string& {
  return no_records_;
}
auto DayBaseConfig::GetStatisticsLabel() const -> const std::string& {
  return statistics_label_;
}
auto DayBaseConfig::GetAllActivitiesLabel() const -> const std::string& {
  return all_activities_label_;
}
auto DayBaseConfig::GetActivityRemarkLabel() const -> const std::string& {
  return activity_remark_label_;
}
auto DayBaseConfig::GetActivityConnector() const -> const std::string& {
  return activity_connector_;
}
auto DayBaseConfig::GetStatisticsItems() const
    -> const std::vector<StatisticItemConfig>& {
  return statistics_items_;
}
auto DayBaseConfig::GetProjectBreakdownLabel() const -> const std::string& {
  return project_breakdown_label_;
}
