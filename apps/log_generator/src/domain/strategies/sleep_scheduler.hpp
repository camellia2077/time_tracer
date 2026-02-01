// domain/strategies/sleep_scheduler.hpp
#ifndef DOMAIN_STRATEGIES_SLEEP_SCHEDULER_H_
#define DOMAIN_STRATEGIES_SLEEP_SCHEDULER_H_

#include <random>

class SleepScheduler {
 public:
  SleepScheduler(bool enabled, std::mt19937& gen);
  void reset_for_new_month();
  bool determine_if_nosleep(int day, int days_in_month);

 private:
  bool enabled_;
  std::mt19937& gen_;
  int current_sequence_length_ = 1;
  int days_into_sequence_ = 0;
  bool is_in_nosleep_block_ = false;
  std::uniform_int_distribution<> nosleep_length_dist_;
  std::uniform_int_distribution<> normal_length_dist_;
};

#endif  // DOMAIN_STRATEGIES_SLEEP_SCHEDULER_H_
