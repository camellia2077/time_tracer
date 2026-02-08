// domain/logic/validator/common/diagnostic.hpp
#ifndef VALIDATOR_COMMON_DIAGNOSTIC_H_
#define VALIDATOR_COMMON_DIAGNOSTIC_H_

#include <optional>
#include <string>

#include "domain/model/source_span.hpp"

namespace validator {

enum class DiagnosticSeverity { kError, kWarning, kInfo };

struct Diagnostic {
  DiagnosticSeverity severity = DiagnosticSeverity::kError;
  std::string code;
  std::string message;
  std::optional<SourceSpan> source_span;
};

}  // namespace validator

#endif  // VALIDATOR_COMMON_DIAGNOSTIC_H_
