// reprocessing/converter/pipelines/GeneratedStatsRules.hpp
#ifndef GENERATED_STATS_RULES_HPP
#define GENERATED_STATS_RULES_HPP

#include "reprocessing/converter/model/InputData.hpp"
#include <initializer_list>

struct StatsRule {
    // [核心修改]
    const char* parent;
    std::initializer_list<const char*> children;
    int GeneratedStats::*member;
};

namespace GeneratedStatsRules {
    constexpr StatsRule rules[] = {
        {"sleep", {}, &GeneratedStats::sleepTime},
        {"exercise", {}, &GeneratedStats::totalExerciseTime},
        {"exercise", {"cardio"}, &GeneratedStats::cardioTime},
        {"exercise", {"anaerobic"}, &GeneratedStats::anaerobicTime},
        {"routine", {"grooming"}, &GeneratedStats::groomingTime},
        {"routine", {"toilet"}, &GeneratedStats::toiletTime},
        {"recreation", {"game"}, &GeneratedStats::gamingTime}
    };
}

#endif // GENERATED_STATS_RULES_HPP