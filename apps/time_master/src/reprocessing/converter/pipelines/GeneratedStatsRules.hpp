// converter/pipelines/GeneratedStatsRules.hpp
#ifndef GENERATED_STATS_RULES_HPP
#define GENERATED_STATS_RULES_HPP

#include <string>
#include <vector>
#include <optional>
#include <initializer_list> // [FIX] Include required header

// Define the statistics rule data structure
struct StatsRule {
    const char* key_name;
    const char* topParent;
    // [FIX] Use std::initializer_list instead of std::vector for constexpr compatibility
    std::initializer_list<const char*> parents; 
};

namespace GeneratedStatsRules {
    // Using constexpr ensures that the rules are determined at compile time
    constexpr StatsRule rules[] = {
        {"sleepTime", "sleep", {}},
        {"totalExerciseTime", "exercise", {}},
        {"cardioTime", "exercise", {"cardio"}},
        {"anaerobicTime", "exercise", {"anaerobic"}},
        {"groomingTime", "routine", {"grooming"}},
        {"toiletTime", "routine", {"toilet"}},
        {"gamingTime", "recreation", {"game"}}
        // Future rules can be added here directly
    };
}

#endif // GENERATED_STATS_RULES_HPP