// domain/insights/models/range_insights_data.hpp
#ifndef DOMAIN_INSIGHTS_MODELS_RANGE_INSIGHTS_DATA_H_
#define DOMAIN_INSIGHTS_MODELS_RANGE_INSIGHTS_DATA_H_

#include <cstdint>
#include <string>
#include <vector>

#include "domain/insights/models/project_tree.hpp"
#include "domain/insights/models/insights_status_value.hpp"

struct RangeInsightsData {
  std::string range_label;
  std::string start_date;
  std::string end_date;
  int requested_days = 0;
  bool has_records = false;
  int matched_day_count = 0;
  int matched_record_count = 0;
  std::int64_t total_duration = 0;
  int actual_days = 0;
  int status_true_days = 0;
  int exercise_true_days = 0;
  int cardio_true_days = 0;
  int anaerobic_true_days = 0;
  std::vector<InsightsStatusValue> statuses;
  bool is_valid = true;

  std::vector<std::pair<std::int64_t, std::int64_t>> project_stats;
  insights::ProjectTree project_tree;
};

#endif  // DOMAIN_INSIGHTS_MODELS_RANGE_INSIGHTS_DATA_H_
