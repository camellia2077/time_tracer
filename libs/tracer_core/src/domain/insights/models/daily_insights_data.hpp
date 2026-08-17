// domain/insights/models/daily_insights_data.hpp
#ifndef DOMAIN_INSIGHTS_MODELS_DAILY_INSIGHTS_DATA_H_
#define DOMAIN_INSIGHTS_MODELS_DAILY_INSIGHTS_DATA_H_

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "domain/model/time_data_models.hpp"
#include "domain/insights/models/activity_aggregate.hpp"
#include "domain/insights/models/project_tree.hpp"
#include "domain/insights/models/insights_status_value.hpp"

struct TimeRecord {
  ActivityRecordKind kind = ActivityRecordKind::kInterval;
  std::int64_t logical_id = 0;
  std::string start_time;
  std::string end_time;
  std::string project_path;
  std::int64_t duration_seconds;
  std::optional<std::string> activityRemark;
};

struct DayMetadata {
  std::string remark = "N/A";
  std::string getup_time = "N/A";
  std::vector<InsightsStatusValue> statuses;
};

struct DailyInsightsData {
  std::string date;
  DayMetadata metadata;
  ActivityAggregate activity;

  std::vector<std::pair<std::int64_t, std::int64_t>> project_stats;
  std::vector<TimeRecord> detailed_records;
  std::map<std::string, std::int64_t> stats;
  insights::ProjectTree project_tree;
};

#endif  // DOMAIN_INSIGHTS_MODELS_DAILY_INSIGHTS_DATA_H_
