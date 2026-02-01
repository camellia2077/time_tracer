// domain/components/event_generator.hpp
#ifndef DOMAIN_COMPONENTS_EVENT_GENERATOR_H_
#define DOMAIN_COMPONENTS_EVENT_GENERATOR_H_

#include <cstddef>
#include <optional>
#include <random>
#include <string>
#include <string_view>
#include <vector>

#include "common/config_types.hpp"

class EventGenerator {
 public:
  EventGenerator(int items_per_day, const std::vector<std::string>& activities,
                 const std::optional<ActivityRemarkConfig>& remark_config,
                 const std::vector<std::string>& wake_keywords,
                 std::mt19937& gen);

  void generate_events_for_day(std::string& log_content, bool is_nosleep_day);

 private:
  int items_per_day_;
  const std::vector<std::string>& common_activities_;
  const std::optional<ActivityRemarkConfig>& remark_config_;
  const std::vector<std::string>& wake_keywords_;
  std::mt19937& gen_;
  std::uniform_int_distribution<> dis_minute_;
  std::uniform_int_distribution<> dis_activity_selector_;
  std::uniform_int_distribution<> dis_wake_keyword_selector_;
  std::bernoulli_distribution should_generate_remark_;
  const std::vector<std::string> remark_delimiters_ = {"//", "#", ";"};
  size_t remark_content_idx_ = 0;
  size_t remark_delimiter_idx_ = 0;
  double slot_size_minutes_;
};

#endif  // DOMAIN_COMPONENTS_EVENT_GENERATOR_H_
