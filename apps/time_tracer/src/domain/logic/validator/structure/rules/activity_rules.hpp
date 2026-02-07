// domain/logic/validator/structure/rules/activity_rules.hpp
#ifndef VALIDATOR_STRUCTURE_RULES_ACTIVITY_RULES_H_
#define VALIDATOR_STRUCTURE_RULES_ACTIVITY_RULES_H_

#include <set>

#include "domain/logic/validator/common/validator_utils.hpp"
#include "domain/model/daily_log.hpp"

namespace validator::structure {

void validateActivityCount(const DailyLog& day, std::set<Error>& errors);

}  // namespace validator::structure

#endif  // VALIDATOR_STRUCTURE_RULES_ACTIVITY_RULES_H_
