// domain/reports/models/daily_report_data.hpp
#ifndef REPORTS_DATA_MODEL_DAILY_REPORT_DATA_H_
#define REPORTS_DATA_MODEL_DAILY_REPORT_DATA_H_

#include <map>
#include <optional>
#include <string>
#include <vector>

#include "domain/reports/models/project_tree.hpp"

struct TimeRecord {
  std::string start_time;
  std::string end_time;
  std::string project_path;
  long long duration_seconds;
  std::optional<std::string> activityRemark;
};

struct DayMetadata {
  std::string status = "N/A";
  std::string sleep = "N/A";
  std::string remark = "N/A";
  std::string getup_time = "N/A";
  std::string exercise = "N/A";
};

struct DailyReportData {
  std::string date;
  DayMetadata metadata;
  long long total_duration = 0;

  std::vector<std::pair<long long, long long>> project_stats;
  std::vector<TimeRecord> detailed_records;
  std::map<std::string, long long> stats;
  reporting::ProjectTree project_tree;
};

#endif  // REPORTS_DATA_MODEL_DAILY_REPORT_DATA_H_
