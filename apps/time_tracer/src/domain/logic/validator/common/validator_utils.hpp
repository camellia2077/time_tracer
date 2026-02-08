// domain/logic/validator/common/validator_utils.hpp
#ifndef VALIDATOR_COMMON_VALIDATOR_UTILS_H_
#define VALIDATOR_COMMON_VALIDATOR_UTILS_H_

#include <optional>
#include <set>
#include <string>
#include <vector>

#include "domain/logic/validator/common/diagnostic.hpp"
#include "domain/model/source_span.hpp"
#include "domain/types/date_check_mode.hpp"

namespace validator {

enum class ErrorType {
  kFileAccess,
  kStructural,
  kLineFormat,
  kTimeDiscontinuity,
  kMissingSleepNight,
  kLogical,
  kDateContinuity,
  kIncorrectDayCountForMonth,
  kSourceRemarkAfterEvent,
  kSourceNoDateAtStart,
  kUnrecognizedActivity,
  kSourceInvalidLineFormat,
  kSourceMissingYearHeader,
  kJsonTooFewActivities,
  kZeroDurationActivity,
  kActivityDurationTooLong
};

struct Error {
  int line_number;
  std::string message;
  ErrorType type;
  std::optional<SourceSpan> source_span;

  auto operator<(const Error& other) const -> bool {
    if (line_number != other.line_number) {
      return line_number < other.line_number;
    }
    if (type != other.type) {
      return type < other.type;
    }
    return message < other.message;
  }
};

void PrintGroupedErrors(const std::string& filename,
                        const std::set<Error>& errors);
void PrintDiagnostics(const std::string& fallback_filename,
                      const std::vector<Diagnostic>& diagnostics);

}  // namespace validator

#endif  // VALIDATOR_COMMON_VALIDATOR_UTILS_H_
