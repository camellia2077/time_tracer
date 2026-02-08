// domain/logic/validator/structure/facade/struct_validator.hpp
#ifndef VALIDATOR_STRUCTURE_FACADE_STRUCT_VALIDATOR_H_
#define VALIDATOR_STRUCTURE_FACADE_STRUCT_VALIDATOR_H_

#include <string>
#include <vector>

#include "domain/logic/validator/common/diagnostic.hpp"
#include "domain/model/daily_log.hpp"
#include "domain/types/date_check_mode.hpp"

namespace validator::structure {

/**
 * @brief Logic Validator operating directly on C++ structures.
 *
 * Designed to replace the JSON-based validator for internal pipeline steps.
 * validates native Structs (DailyLog) directly to avoid the overhead of
 * converting objects to JSON just for validation purposes.
 */
class StructValidator {
 public:
  explicit StructValidator(DateCheckMode mode = DateCheckMode::kNone);

  auto Validate(const std::string& filename, const std::vector<DailyLog>& days,
                std::vector<Diagnostic>& diagnostics) -> bool;

 private:
  DateCheckMode date_check_mode_;
};

}  // namespace validator::structure

#endif  // VALIDATOR_STRUCTURE_FACADE_STRUCT_VALIDATOR_H_
