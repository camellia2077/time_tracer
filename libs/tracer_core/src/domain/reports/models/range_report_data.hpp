// domain/reports/models/range_report_data.hpp
#ifndef DOMAIN_REPORTS_MODELS_RANGE_REPORT_DATA_H_
#define DOMAIN_REPORTS_MODELS_RANGE_REPORT_DATA_H_

#include <cstdint>
#include <string>
#include <vector>

#include "domain/reports/models/project_tree.hpp"

struct RangeReportData {
  std::string range_label;
  std::string start_date;
  std::string end_date;
  int requested_days = 0;
  std::int64_t total_duration = 0;
  int actual_days = 0;
  int status_true_days = 0;
  int wake_anchor_true_days = 0;
  int exercise_true_days = 0;
  int cardio_true_days = 0;
  int anaerobic_true_days = 0;
  bool is_valid = true;

  std::vector<std::pair<std::int64_t, std::int64_t>> project_stats;
  reporting::ProjectTree project_tree;
};

#endif  // DOMAIN_REPORTS_MODELS_RANGE_REPORT_DATA_H_
