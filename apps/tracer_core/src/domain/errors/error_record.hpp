// domain/errors/error_record.hpp
#ifndef DOMAIN_ERRORS_ERROR_RECORD_H_
#define DOMAIN_ERRORS_ERROR_RECORD_H_

#include <string>

namespace tracer_core::domain::errors {

// Stable string identifier for an error, e.g. "validation.time.discontinuity".
// Codes are dot-style lowercase; defined in error_codes.hpp.
using ErrorCode = std::string;

enum class ErrorSeverity {
  kInfo,
  kWarn,
  kError,
};

// Logical error record model for the time_tracer core.
// Represents a single error occurrence as collected within one run session.
//
// Fields:
//   code          - Stable machine-readable identifier (see ErrorCode).
//   severity      - Diagnostic severity level.
//   message       - Short human-readable description.
//   details       - Optional technical detail (stack hint, raw line, etc.).
//   source        - Optional origin location (file path, module, line number).
//   timestamp_utc - ISO-8601 UTC timestamp string of when the error occurred.
//   run_id        - Identifier for the run session (derived from ISO
//   timestamp).
struct ErrorRecord {
  ErrorCode code;
  ErrorSeverity severity = ErrorSeverity::kError;
  std::string message;
  std::string details;  // optional
  std::string source;   // optional: "file:line" or module name
  std::string timestamp_utc;
  std::string run_id;
};

}  // namespace tracer_core::domain::errors

#endif  // DOMAIN_ERRORS_ERROR_RECORD_H_
