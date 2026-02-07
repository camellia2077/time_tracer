// domain/logic/validator/structure/rules/date_rules.hpp
#ifndef VALIDATOR_STRUCTURE_RULES_DATE_RULES_H_
#define VALIDATOR_STRUCTURE_RULES_DATE_RULES_H_

#include <set>
#include <vector>

#include "domain/logic/validator/common/validator_utils.hpp"
#include "domain/model/daily_log.hpp"
#include "domain/types/date_check_mode.hpp"

namespace validator::structure {

void validateDateContinuity(const std::vector<DailyLog>& days,
                            std::set<Error>& errors, DateCheckMode mode);

}  // namespace validator::structure

#endif  // VALIDATOR_STRUCTURE_RULES_DATE_RULES_H_
