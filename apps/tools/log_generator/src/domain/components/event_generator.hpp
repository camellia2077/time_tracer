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
#include "domain/model/generated_event.hpp"

class EventGenerator {
 public:
  EventGenerator(int items_per_day,
                 const std::vector<ActivityTokenVariant>& activities,
                 const std::optional<ActivityRemarkConfig>& remark_config,
                 const std::vector<std::string>& wake_keywords,
                 EventStyle event_style,
                 bool enable_explicit_sleep,
                 std::mt19937& gen);

  void reset_for_new_month();
  auto generate_events_for_day(bool is_nosleep_day) -> std::vector<GeneratedEvent>;

 private:
  static auto to_minute_of_day(int logical_minutes) -> int;
  auto select_wake_time_minutes(int day_start_minutes, int day_end_minutes,
                                int non_wake_event_count) -> int;
  auto take_wake_seconds(int day_start_minutes, int day_end_minutes,
                         int non_wake_event_count) -> int;
  auto build_activity_start_seconds(int start_minutes, int end_minutes,
                                    int event_count,
                                    bool include_initial_start,
                                    std::optional<int> initial_start_seconds =
                                        std::nullopt)
      -> std::vector<int>;
  auto build_day_events(int day_start_minutes, int day_end_minutes,
                        bool is_nosleep_day) -> std::vector<GeneratedEvent>;
  auto build_interval_events(int day_start_minutes, int day_end_minutes,
                             bool is_nosleep_day)
      -> std::vector<GeneratedEvent>;
  auto build_mixed_events(int day_start_minutes, int day_end_minutes,
                          bool is_nosleep_day) -> std::vector<GeneratedEvent>;
  auto build_explicit_sleep_events() -> std::vector<GeneratedEvent>;
  auto build_point_event(int logical_seconds, std::string_view activity_token,
                         std::optional<std::string> remark_suffix) const
      -> GeneratedEvent;
  auto build_interval_event(
      int start_seconds, int end_seconds, std::string_view activity_token,
      std::optional<std::string> remark_suffix) const -> GeneratedEvent;
  auto maybe_build_remark_suffix() -> std::optional<std::string>;
  auto resolve_activity_token(const ActivityTokenVariant& activity) -> std::string_view;

  int items_per_day_;
  EventStyle event_style_;
  bool enable_explicit_sleep_;
  const std::vector<ActivityTokenVariant>& common_activities_;
  std::optional<ActivityRemarkConfig> remark_config_;
  const std::vector<std::string>& wake_keywords_;
  std::mt19937& gen_;
  std::uniform_int_distribution<> dis_minute_;
  std::uniform_int_distribution<> dis_second_;
  std::uniform_int_distribution<> dis_activity_selector_;
  std::uniform_int_distribution<> dis_remark_content_selector_;
  std::uniform_int_distribution<> dis_remark_lines_;
  std::uniform_int_distribution<> dis_wake_keyword_selector_;
  std::uniform_int_distribution<> dis_budget_jitter_minutes_;
  std::bernoulli_distribution should_generate_mixed_point_;
  std::bernoulli_distribution should_generate_explicit_sleep_;
  std::bernoulli_distribution should_use_canonical_token_;
  std::bernoulli_distribution should_generate_remark_;
  std::vector<int> activity_candidates_;
  int previous_day_last_minutes_ = 0;
  int carry_error_minutes_ = 0;
  bool after_explicit_sleep_ = false;
  std::optional<int> forced_wake_seconds_;
};

#endif  // DOMAIN_COMPONENTS_EVENT_GENERATOR_H_
