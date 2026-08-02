// domain/reports/models/daily_report_data.hpp
#ifndef DOMAIN_REPORTS_MODELS_DAILY_REPORT_DATA_H_
#define DOMAIN_REPORTS_MODELS_DAILY_REPORT_DATA_H_

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "domain/model/time_data_models.hpp"
#include "domain/reports/models/project_tree.hpp"

struct TimeRecord {
  ActivityRecordKind kind = ActivityRecordKind::kInterval;
  std::int64_t logical_id = 0;
  std::string start_time;
  std::string end_time;
  std::string project_path;
  std::int64_t duration_seconds;
  std::optional<std::string> activityRemark;
};

struct DailyStatusValue {
  std::string id;
  std::string label;
  bool value = false;
};

struct DayMetadata {
  std::string remark = "N/A";
  std::string getup_time = "N/A";
  std::vector<DailyStatusValue> statuses;
};

struct DailyReportData {
  std::string date;
  DayMetadata metadata;
  std::int64_t total_duration = 0;
  int activity_count = 0;

  std::vector<std::pair<std::int64_t, std::int64_t>> project_stats;
  std::vector<TimeRecord> detailed_records;
  std::map<std::string, std::int64_t> stats;
  reporting::ProjectTree project_tree;
};

#endif  // DOMAIN_REPORTS_MODELS_DAILY_REPORT_DATA_H_
