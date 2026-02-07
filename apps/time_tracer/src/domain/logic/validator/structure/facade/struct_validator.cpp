// domain/logic/validator/structure/facade/struct_validator.cpp
#include "domain/logic/validator/structure/facade/struct_validator.hpp"

#include "domain/logic/validator/structure/rules/activity_rules.hpp"
#include "domain/logic/validator/structure/rules/date_rules.hpp"

namespace validator::structure {

StructValidator::StructValidator(DateCheckMode mode) : date_check_mode_(mode) {}

auto StructValidator::Validate(const std::string& /*filename*/,
                               const std::vector<DailyLog>& days,
                               std::set<Error>& errors) -> bool {
  bool is_valid = true;

  // 1. Validate Date Continuity (Structure)
  validateDateContinuity(days, errors, date_check_mode_);

  // 2. Validate content of each day (Activity rules)
  for (const auto& day : days) {
    validateActivityCount(day, errors);
  }

  if (!errors.empty()) {
    is_valid = false;
  }

  return is_valid;
}

}  // namespace validator::structure
