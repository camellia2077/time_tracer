// reports/data/model/range_report_data.hpp
#ifndef REPORTS_DATA_MODEL_RANGE_REPORT_DATA_H_
#define REPORTS_DATA_MODEL_RANGE_REPORT_DATA_H_

#include <string>
#include <vector>

#include "reports/data/model/project_tree.hpp"

struct RangeReportData {
  std::string range_label;
  std::string start_date;
  std::string end_date;
  int requested_days = 0;
  long long total_duration = 0;
  int actual_days = 0;
  bool is_valid = true;

  std::vector<std::pair<std::string, long long>> records;

  // [新增] 用于存储 BaseQuerier 聚合查询的结果(Project ID -> Duration)
  std::vector<std::pair<long long, long long>> project_stats;

  reporting::ProjectTree project_tree;
};

#endif  // REPORTS_DATA_MODEL_RANGE_REPORT_DATA_H_
