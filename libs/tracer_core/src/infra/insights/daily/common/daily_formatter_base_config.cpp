// infra/insights/daily/common/daily_formatter_base_config.cpp
#include "infra/insights/daily/common/daily_formatter_base_config.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {

auto BuildStatisticItemTree(const InsightsStatisticsItem& item)
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

DailyFormatterBaseConfig::DailyFormatterBaseConfig(
    const DailyInsightsLabels& labels,
    const std::vector<InsightsStatisticsItem>& statistics_items)
    : statistics_items_(BuildStatisticsItems(statistics_items)) {
  LoadBaseConfig(labels);
}

auto DailyFormatterBaseConfig::BuildStatisticsItems(
    const std::vector<InsightsStatisticsItem>& statistics_items)
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

void DailyFormatterBaseConfig::LoadBaseConfig(
    const DailyInsightsLabels& labels) {
  total_time_label_ = labels.total_time_label;
  activity_count_label_ = labels.activity_count_label;
  status_count_unit_ = labels.status_count_unit;
  custom_section_label_ = labels.custom_section_label;
  summary_section_label_ = labels.summary_section_label;
  period_label_ = labels.period_label;
  getup_time_label_ = labels.getup_time_label;
  remark_label_ = labels.remark_label;
  no_records_ = labels.no_records_message;
  statistics_label_ = labels.statistics_label;
  all_activities_label_ = labels.all_activities_label;
  activity_remark_label_ = labels.activity_remark_label;
  activity_connector_ = labels.activity_connector;
  project_breakdown_label_ = labels.project_breakdown_label;
}

auto DailyFormatterBaseConfig::GetTotalTimeLabel() const -> const std::string& {
  return total_time_label_;
}
auto DailyFormatterBaseConfig::GetActivityCountLabel() const
    -> const std::string& {
  return activity_count_label_;
}
auto DailyFormatterBaseConfig::GetStatusCountUnit() const
    -> const std::string& {
  return status_count_unit_;
}
auto DailyFormatterBaseConfig::GetCustomSectionLabel() const
    -> const std::string& {
  return custom_section_label_;
}
auto DailyFormatterBaseConfig::GetSummarySectionLabel() const
    -> const std::string& {
  return summary_section_label_;
}
auto DailyFormatterBaseConfig::GetPeriodLabel() const -> const std::string& {
  return period_label_;
}
auto DailyFormatterBaseConfig::GetGetupTimeLabel() const -> const std::string& {
  return getup_time_label_;
}
auto DailyFormatterBaseConfig::GetRemarkLabel() const -> const std::string& {
  return remark_label_;
}
auto DailyFormatterBaseConfig::GetNoRecords() const -> const std::string& {
  return no_records_;
}
auto DailyFormatterBaseConfig::GetStatisticsLabel() const
    -> const std::string& {
  return statistics_label_;
}
auto DailyFormatterBaseConfig::GetAllActivitiesLabel() const
    -> const std::string& {
  return all_activities_label_;
}
auto DailyFormatterBaseConfig::GetActivityRemarkLabel() const
    -> const std::string& {
  return activity_remark_label_;
}
auto DailyFormatterBaseConfig::GetActivityConnector() const
    -> const std::string& {
  return activity_connector_;
}
auto DailyFormatterBaseConfig::GetStatisticsItems() const
    -> const std::vector<StatisticItemConfig>& {
  return statistics_items_;
}
auto DailyFormatterBaseConfig::GetProjectBreakdownLabel() const
    -> const std::string& {
  return project_breakdown_label_;
}
