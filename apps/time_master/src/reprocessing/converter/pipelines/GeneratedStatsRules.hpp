// reprocessing/converter/pipelines/GeneratedStatsRules.hpp
#ifndef GENERATED_STATS_RULES_HPP
#define GENERATED_STATS_RULES_HPP

#include "reprocessing/converter/model/InputData.hpp"
#include <initializer_list>

struct StatsRule {
    // --- [核心修改] ---
    // 使用一个完整的路径字符串进行匹配
    const char* match_path;
    int GeneratedStats::*member;
};

namespace GeneratedStatsRules {
    constexpr StatsRule rules[] = {
        {"exercise", &GeneratedStats::totalExerciseTime},
        {"exercise_cardio", &GeneratedStats::cardioTime},
        {"exercise_anaerobic", &GeneratedStats::anaerobicTime},
        {"routine_grooming", &GeneratedStats::groomingTime},
        {"routine_toilet", &GeneratedStats::toiletTime},
        {"recreation_game", &GeneratedStats::gamingTime}
    };
}

#endif // GENERATED_STATS_RULES_HPP