// converter/pipelines/GeneratedStatsRules.hpp
#ifndef GENERATED_STATS_RULES_HPP
#define GENERATED_STATS_RULES_HPP

#include <string>
#include <vector>
#include <optional>

// 定义统计规则的数据结构
struct StatsRule {
    const char* key_name;
    const char* topParent;
    std::vector<const char*> parents; // 使用vector来允许多个parent条件
};

namespace GeneratedStatsRules {
    // 使用constexpr确保规则在编译时确定
    constexpr StatsRule rules[] = {
        {"sleepTime", "sleep", {}},
        {"totalExerciseTime", "exercise", {}},
        {"cardioTime", "exercise", {"cardio"}},
        {"anaerobicTime", "exercise", {"anaerobic"}},
        {"groomingTime", "routine", {"grooming"}},
        {"toiletTime", "routine", {"toilet"}},
        {"gamingTime", "recreation", {"game"}}
        // 未来若有新规则，可直接在此处添加
    };
}

#endif // GENERATED_STATS_RULES_HPP