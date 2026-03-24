// infra/reporting/daily/common/day_base_config.cpp
#include "infra/reporting/daily/common/day_base_config.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {

auto BuildStatisticItemTree(const ReportStatisticsItem& item)
    -> StatisticItemConfig {
  StatisticItemConfig config_item{};
  config_item.label = item.label;
  config_item.db_column = item.db_column;
  config_item.show = item.show;
  config_item.sub_items.reserve(item.sub_items.size());
  for (const auto& child : item.sub_items) {
    config_item.sub_items.push_back(BuildStatisticItemTree(child));
  }
  return config_item;
}

}  // namespace

DayBaseConfig::DayBaseConfig(
    const DailyReportLabels& labels,
    const std::vector<ReportStatisticsItem>& statistics_items)
    : statistics_items_(BuildStatisticsItems(statistics_items)) {
  LoadBaseConfig(labels);
}

auto DayBaseConfig::BuildStatisticsItems(
    const std::vector<ReportStatisticsItem>& statistics_items)
    -> std::vector<StatisticItemConfig> {
  if (statistics_items.empty()) {
    return {};
  }

  std::vector<StatisticItemConfig> tree_items;
  tree_items.reserve(statistics_items.size());
  for (const auto& item : statistics_items) {
    tree_items.push_back(BuildStatisticItemTree(item));
  }
  return tree_items;
}

void DayBaseConfig::LoadBaseConfig(const DailyReportLabels& labels) {
  title_prefix_ = labels.report_title_prefix;
  date_label_ = labels.date_label;
  total_time_label_ = labels.total_time_label;
  status_label_ = labels.status_label;
  wake_anchor_label_ = labels.wake_anchor_label;
  getup_time_label_ = labels.getup_time_label;
  remark_label_ = labels.remark_label;
  exercise_label_ = labels.exercise_label;
  no_records_ = labels.no_records_message;
  statistics_label_ = labels.statistics_label;
  all_activities_label_ = labels.all_activities_label;
  activity_remark_label_ = labels.activity_remark_label;
  activity_connector_ = labels.activity_connector;
  project_breakdown_label_ = labels.project_breakdown_label;
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
auto DayBaseConfig::GetWakeAnchorLabel() const -> const std::string& {
  return wake_anchor_label_;
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
