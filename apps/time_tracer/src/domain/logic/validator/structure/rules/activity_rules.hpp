// domain/logic/validator/structure/rules/activity_rules.hpp
#ifndef VALIDATOR_STRUCTURE_RULES_ACTIVITY_RULES_H_
#define VALIDATOR_STRUCTURE_RULES_ACTIVITY_RULES_H_

#include <vector>

#include "domain/logic/validator/common/diagnostic.hpp"
#include "domain/model/daily_log.hpp"

namespace validator::structure {

void ValidateActivityCount(const DailyLog& day,
                           std::vector<Diagnostic>& diagnostics);
void ValidateActivityDuration(const DailyLog& day,
                              std::vector<Diagnostic>& diagnostics);

}  // namespace validator::structure

#endif  // VALIDATOR_STRUCTURE_RULES_ACTIVITY_RULES_H_
