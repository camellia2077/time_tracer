// reprocessing/converter/pipelines/GeneratedStatsRules.hpp
#ifndef GENERATED_STATS_RULES_HPP
#define GENERATED_STATS_RULES_HPP

#include "reprocessing/converter/model/InputData.hpp" // [新增] 引入InputData来访问GeneratedStats
#include <initializer_list>

// [核心修改] 规则结构体现在包含一个指向GeneratedStats成员的指针
struct StatsRule {
    const char* topParent;
    std::initializer_list<const char*> parents;
    int GeneratedStats::*member; // 成员指针
};

namespace GeneratedStatsRules {
    // 使用constexpr确保规则在编译时确定
    // [核心修改] 每个规则现在直接关联到GeneratedStats的成员
    constexpr StatsRule rules[] = {
        {"sleep", {}, &GeneratedStats::sleepTime},
        {"exercise", {}, &GeneratedStats::totalExerciseTime},
        {"exercise", {"cardio"}, &GeneratedStats::cardioTime},
        {"exercise", {"anaerobic"}, &GeneratedStats::anaerobicTime},
        {"routine", {"grooming"}, &GeneratedStats::groomingTime},
        {"routine", {"toilet"}, &GeneratedStats::toiletTime},
        {"recreation", {"game"}, &GeneratedStats::gamingTime}
        // 未来若有新规则，只需在此处添加一行即可
        // 例如: {"routine", {"housework"}, &GeneratedStats::houseworkTime}
    };
}

#endif // GENERATED_STATS_RULES_HPP