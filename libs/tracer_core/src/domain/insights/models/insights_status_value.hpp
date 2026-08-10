// domain/insights/models/insights_status_value.hpp
#ifndef DOMAIN_INSIGHTS_MODELS_INSIGHTS_STATUS_VALUE_H_
#define DOMAIN_INSIGHTS_MODELS_INSIGHTS_STATUS_VALUE_H_

#include <cstdint>
#include <string>

struct InsightsStatusValue {
  std::string id;
  std::string label;
  int occurrence_count = 0;
  std::int64_t total_duration = 0;
};

#endif  // DOMAIN_INSIGHTS_MODELS_INSIGHTS_STATUS_VALUE_H_
