// domain/logic/converter/convert/core/stats_rules.hpp
#ifndef CONVERTER_CONVERT_CORE_STATS_RULES_H_
#define CONVERTER_CONVERT_CORE_STATS_RULES_H_

#include <array>

#include "domain/model/daily_log.hpp"

struct StatsRule {
  const char* match_path;
  int ActivityStats::* member;
};

namespace StatsRules {
constexpr std::array kRules = {
    StatsRule{.match_path = "study", .member = &ActivityStats::study_time},
    StatsRule{.match_path = "exercise",
              .member = &ActivityStats::total_exercise_time},
    StatsRule{.match_path = "exercise_cardio",
              .member = &ActivityStats::cardio_time},
    StatsRule{.match_path = "exercise_anaerobic",
              .member = &ActivityStats::anaerobic_time},
    StatsRule{.match_path = "routine_grooming",
              .member = &ActivityStats::grooming_time},
    StatsRule{.match_path = "routine_toilet",
              .member = &ActivityStats::toilet_time},
    StatsRule{.match_path = "recreation_game",
              .member = &ActivityStats::gaming_time},
    StatsRule{.match_path = "recreation",
              .member = &ActivityStats::recreation_time},
    StatsRule{.match_path = "recreation_zhihu",
              .member = &ActivityStats::recreation_zhihu_time},
    StatsRule{.match_path = "recreation_bilibili",
              .member = &ActivityStats::recreation_bilibili_time},
    StatsRule{.match_path = "recreation_douyin",
              .member = &ActivityStats::recreation_douyin_time}};
}  // namespace StatsRules

#endif  // CONVERTER_CONVERT_CORE_STATS_RULES_H_
