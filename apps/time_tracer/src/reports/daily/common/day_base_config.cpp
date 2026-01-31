// reports/daily/common/day_base_config.cpp
#include "day_base_config.hpp"

#include <stdexcept>
#include <utility>

static auto ParseStatisticsItemsRecursive(const toml::array* arr)
    -> std::vector<StatisticItemConfig> {
  std::vector<StatisticItemConfig> items;
  if (arr == nullptr) {
    return items;
  }

  for (const auto& node : *arr) {
    if (!node.is_table()) {
      continue;
    }
    const auto& item_tbl = *node.as_table();

    StatisticItemConfig item;
    item.label = item_tbl["label"].value_or<std::string>("");
    item.db_column = item_tbl["db_column"].value_or<std::string>("");
    item.show = item_tbl["show"].value_or(true);

    if (const toml::array* sub_arr = item_tbl["sub_items"].as_array()) {
      item.sub_items = ParseStatisticsItemsRecursive(sub_arr);
    }

    items.push_back(item);
  }
  return items;
}

// [修改] 构造函数
DayBaseConfig::DayBaseConfig(toml::table config)
    : config_table_(std::move(config)) {
  load_base_config();
}

void DayBaseConfig::load_base_config() {
  // [修改] 使用 TOML++ API
  if (auto val = config_table_["title_prefix"].value<std::string>()) {
    title_prefix_ = *val;
  } else {
    // 兼容 report_title
    title_prefix_ = config_table_["report_title"].value_or("Daily Report for");
  }

  date_label_ = config_table_["date_label"].value_or("");
  total_time_label_ = config_table_["total_time_label"].value_or("");
  status_label_ = config_table_["status_label"].value_or("Status");
  sleep_label_ = config_table_["sleep_label"].value_or("Sleep");
  getup_time_label_ = config_table_["getup_time_label"].value_or("Getup Time");
  remark_label_ = config_table_["remark_label"].value_or("Remark");
  exercise_label_ = config_table_["exercise_label"].value_or("Exercise");

  no_records_ = config_table_["no_records_message"].value_or("No records.");

  statistics_label_ = config_table_["statistics_label"].value_or("Statistics");
  all_activities_label_ =
      config_table_["all_activities_label"].value_or("All Activities");
  activity_remark_label_ =
      config_table_["activity_remark_label"].value_or("Remark");
  activity_connector_ = config_table_["activity_connector"].value_or("->");

  project_breakdown_label_ =
      config_table_["project_breakdown_label"].value_or("Project Breakdown");

  if (const toml::array* arr = config_table_["statistics_items"].as_array()) {
    statistics_items_ = ParseStatisticsItemsRecursive(arr);
  }
}

// Getters 保持不变
auto DayBaseConfig::get_title_prefix() const -> const std::string& {
  return title_prefix_;
}
auto DayBaseConfig::get_date_label() const -> const std::string& {
  return date_label_;
}
auto DayBaseConfig::get_total_time_label() const -> const std::string& {
  return total_time_label_;
}
auto DayBaseConfig::get_status_label() const -> const std::string& {
  return status_label_;
}
auto DayBaseConfig::get_sleep_label() const -> const std::string& {
  return sleep_label_;
}
auto DayBaseConfig::get_getup_time_label() const -> const std::string& {
  return getup_time_label_;
}
auto DayBaseConfig::get_remark_label() const -> const std::string& {
  return remark_label_;
}
auto DayBaseConfig::get_exercise_label() const -> const std::string& {
  return exercise_label_;
}
auto DayBaseConfig::get_no_records() const -> const std::string& {
  return no_records_;
}
auto DayBaseConfig::get_statistics_label() const -> const std::string& {
  return statistics_label_;
}
auto DayBaseConfig::get_all_activities_label() const -> const std::string& {
  return all_activities_label_;
}
auto DayBaseConfig::get_activity_remark_label() const -> const std::string& {
  return activity_remark_label_;
}
auto DayBaseConfig::get_activity_connector() const -> const std::string& {
  return activity_connector_;
}
auto DayBaseConfig::get_statistics_items() const
    -> const std::vector<StatisticItemConfig>& {
  return statistics_items_;
}
auto DayBaseConfig::get_project_breakdown_label() const -> const std::string& {
  return project_breakdown_label_;
}