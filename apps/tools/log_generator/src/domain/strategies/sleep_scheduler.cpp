// domain/strategies/sleep_scheduler.cpp
#include "domain/strategies/sleep_scheduler.hpp"

#include <algorithm>

namespace {
constexpr double kMinProbability = 0.0;
constexpr double kMaxProbability = 1.0;
}  // namespace

SleepScheduler::SleepScheduler(bool enabled, double nosleep_probability,
                               std::mt19937& gen)
    : enabled_(enabled),
      gen_(gen),
      nosleep_length_dist_(1, 3),
      normal_length_dist_(2, 3),
      enter_nosleep_dist_(
          std::clamp(nosleep_probability, kMinProbability, kMaxProbability)) {}

void SleepScheduler::reset_for_new_month() {
  if (enabled_) {
    // 原逻辑：每月第1天强制重置
    is_in_nosleep_block_ = false;
    days_into_sequence_ = 1;
    current_sequence_length_ = normal_length_dist_(gen_);
  }
}

auto SleepScheduler::determine_if_nosleep(int day, int days_in_month) -> bool {
  if (!enabled_) {
    return false;
  }

  // 原逻辑：第1天强制 false (已经在 reset_for_new_month
  // 处理了初始状态，这里只需返回当前判定)
  if (day == 1) {
    return false;
  }

  // 原逻辑：最后一天强制 false 并重置
  if (day == days_in_month) {
    is_in_nosleep_block_ = false;
    days_into_sequence_ = 1;
    current_sequence_length_ = normal_length_dist_(gen_);
    return false;
  }

  // 核心状态机推进逻辑
  days_into_sequence_++;
  bool is_nosleep = false;

  if (is_in_nosleep_block_) {
    is_nosleep = true;
    if (days_into_sequence_ >= current_sequence_length_) {
      is_in_nosleep_block_ = false;
      days_into_sequence_ = 0;
      current_sequence_length_ = normal_length_dist_(gen_);
    }
  } else {
    is_nosleep = false;
    if (days_into_sequence_ >= current_sequence_length_) {
      days_into_sequence_ = 0;
      if (enter_nosleep_dist_(gen_)) {
        is_in_nosleep_block_ = true;
        current_sequence_length_ = nosleep_length_dist_(gen_);
      } else {
        is_in_nosleep_block_ = false;
        current_sequence_length_ = normal_length_dist_(gen_);
      }
    }
  }

  return is_nosleep;
}
