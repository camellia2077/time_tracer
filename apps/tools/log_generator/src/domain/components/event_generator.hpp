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

  void reset_for_new_month();
  void generate_events_for_day(std::string& log_content, bool is_nosleep_day);

 private:
  static auto to_minute_of_day(int logical_minutes) -> int;
  void append_event_line(std::string& log_content, int logical_minutes,
                         std::string_view text) const;
  auto select_wake_time_minutes(int day_start_minutes, int day_end_minutes,
                                int non_wake_event_count) -> int;
  auto build_event_minutes(int start_minutes_exclusive, int end_minutes_inclusive,
                           int event_count) -> std::vector<int>;

  int items_per_day_;
  const std::vector<std::string>& common_activities_;
  const std::optional<ActivityRemarkConfig>& remark_config_;
  const std::vector<std::string>& wake_keywords_;
  std::mt19937& gen_;
  std::uniform_int_distribution<> dis_minute_;
  std::uniform_int_distribution<> dis_activity_selector_;
  std::uniform_int_distribution<> dis_wake_keyword_selector_;
  std::uniform_int_distribution<> dis_budget_jitter_minutes_;
  std::bernoulli_distribution should_generate_remark_;
  std::vector<int> activity_candidates_;
  const std::vector<std::string> remark_delimiters_ = {"//", "#", ";"};
  size_t remark_content_idx_ = 0;
  size_t remark_delimiter_idx_ = 0;
  int previous_day_last_minutes_ = 0;
  int carry_error_minutes_ = 0;
};

#endif  // DOMAIN_COMPONENTS_EVENT_GENERATOR_H_
