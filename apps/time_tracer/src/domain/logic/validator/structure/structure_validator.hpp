// domain/logic/validator/structure/structure_validator.hpp
#ifndef DOMAIN_LOGIC_VALIDATOR_STRUCTURE_STRUCTURE_VALIDATOR_H_
#define DOMAIN_LOGIC_VALIDATOR_STRUCTURE_STRUCTURE_VALIDATOR_H_

#include <string>
#include <vector>

#include "domain/logic/validator/common/diagnostic.hpp"
#include "domain/model/daily_log.hpp"
#include "domain/types/date_check_mode.hpp"

namespace validator::structure {

class StructValidator {
 public:
  explicit StructValidator(DateCheckMode mode = DateCheckMode::kNone);

  auto Validate(const std::string& filename, const std::vector<DailyLog>& days,
                std::vector<Diagnostic>& diagnostics) -> bool;

 private:
  DateCheckMode date_check_mode_;
};

}  // namespace validator::structure

#endif  // DOMAIN_LOGIC_VALIDATOR_STRUCTURE_STRUCTURE_VALIDATOR_H_
