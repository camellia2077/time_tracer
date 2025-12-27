// reprocessing/converter/pipelines/StatsRules.hpp
#ifndef STATS_RULES_HPP
#define STATS_RULES_HPP

#include "reprocessing/converter/model/InputData.hpp"
#include <initializer_list>

struct StatsRule {
    // --- [核心修改] ---
    // 使用一个完整的路径字符串进行匹配
    const char* match_path;
    int GeneratedStats::*member;
};

namespace StatsRules {
    constexpr StatsRule rules[] = {
        {"exercise", &GeneratedStats::totalExerciseTime},
        {"exercise_cardio", &GeneratedStats::cardioTime},
        {"exercise_anaerobic", &GeneratedStats::anaerobicTime},
        {"routine_grooming", &GeneratedStats::groomingTime},
        {"routine_toilet", &GeneratedStats::toiletTime},
        {"recreation_game", &GeneratedStats::gamingTime},
        {"recreation", &GeneratedStats::recreationTime},
        {"recreation_zhihu", &GeneratedStats::recreationZhihuTime},
        {"recreation_bilibili", &GeneratedStats::recreationBilibiliTime},
        {"recreation_douyin", &GeneratedStats::recreationDouyinTime}
    };
}

#endif // STATS_RULES_HPP