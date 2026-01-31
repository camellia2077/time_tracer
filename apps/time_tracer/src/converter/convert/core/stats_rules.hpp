// converter/convert/core/stats_rules.hpp
#ifndef CONVERTER_CONVERT_CORE_STATS_RULES_H_
#define CONVERTER_CONVERT_CORE_STATS_RULES_H_

#include <initializer_list>

#include "domain/model/daily_log.hpp"

struct StatsRule {
  const char* match_path;
  int ActivityStats::* member;
};

namespace StatsRules {
constexpr StatsRule rules[] = {
    {"study", &ActivityStats::study_time},
    {"exercise", &ActivityStats::total_exercise_time},
    {"exercise_cardio", &ActivityStats::cardio_time},
    {"exercise_anaerobic", &ActivityStats::anaerobic_time},
    {"routine_grooming", &ActivityStats::grooming_time},
    {"routine_toilet", &ActivityStats::toilet_time},
    {"recreation_game", &ActivityStats::gaming_time},
    {"recreation", &ActivityStats::recreation_time},
    {"recreation_zhihu", &ActivityStats::recreation_zhihu_time},
    {"recreation_bilibili", &ActivityStats::recreation_bilibili_time},
    {"recreation_douyin", &ActivityStats::recreation_douyin_time}};
}

#endif  // CONVERTER_CONVERT_CORE_STATS_RULES_H_
