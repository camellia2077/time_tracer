// converter/convert/pipelines/StatsRules.hpp
#ifndef STATS_RULES_HPP
#define STATS_RULES_HPP

#include "converter/convert/model/InputData.hpp"
#include <initializer_list>

struct StatsRule {
    const char* match_path;
    // [核心修改] 指向 ActivityStats 的成员
    int ActivityStats::*member;
};

namespace StatsRules {
    // [核心修改] 字段名变更适配 (CamelCase -> snake_case)
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
        {"recreation_douyin", &ActivityStats::recreation_douyin_time}
    };
}

#endif // STATS_RULES_HPP