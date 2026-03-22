// domain/logic/validator/structure/structure_validator.hpp
#ifndef DOMAIN_LOGIC_VALIDATOR_STRUCTURE_STRUCTURE_VALIDATOR_H_
#define DOMAIN_LOGIC_VALIDATOR_STRUCTURE_STRUCTURE_VALIDATOR_H_

#include <unordered_set>
#include <string>
#include <vector>

#include "domain/logic/validator/common/diagnostic.hpp"
#include "domain/model/daily_log.hpp"
#include "domain/types/date_check_mode.hpp"

namespace validator::structure {

class StructValidator {
 public:
  explicit StructValidator(DateCheckMode mode = DateCheckMode::kNone,
                           std::vector<std::string> wake_keywords = {});

  auto Validate(const std::string& filename, const std::vector<DailyLog>& days,
                std::vector<Diagnostic>& diagnostics) -> bool;

 private:
  DateCheckMode date_check_mode_;
  std::unordered_set<std::string> wake_keywords_;
};

}  // namespace validator::structure

#endif  // DOMAIN_LOGIC_VALIDATOR_STRUCTURE_STRUCTURE_VALIDATOR_H_
