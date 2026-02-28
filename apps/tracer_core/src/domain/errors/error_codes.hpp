// domain/errors/error_codes.hpp
#ifndef DOMAIN_ERRORS_ERROR_CODES_H_
#define DOMAIN_ERRORS_ERROR_CODES_H_

#include <string_view>

// Stable dot-style error code constants for time_tracer core.
//
// Naming convention: domain.category.detail (all lowercase).
// Codes are stable identifiers — error messages may change, codes should not.
// When adding new codes, update docs/time_tracer/core/contracts/error-codes.md
// as well.
//
// Usage:
//   #include "domain/errors/error_codes.hpp"
//   using namespace tracer_core::domain::errors::codes;
//   ErrorRecord rec{ .code = std::string(validation::kTimeDiscontinuity), ...
//   };

namespace tracer_core::domain::errors::codes {

// ---------------------------------------------------------------------------
// validation.*   Source data / logical validation errors
// ---------------------------------------------------------------------------
namespace validation {

// Source file format
inline constexpr std::string_view kSourceRemarkAfterEvent =
    "validation.source.remark_after_event";
inline constexpr std::string_view kSourceNoDateAtStart =
    "validation.source.no_date_at_start";
inline constexpr std::string_view kSourceInvalidLineFormat =
    "validation.source.invalid_line_format";
inline constexpr std::string_view kSourceMissingYearHeader =
    "validation.source.missing_year_header";
inline constexpr std::string_view kSourceUnrecognizedActivity =
    "validation.source.unrecognized_activity";

// Date logic
inline constexpr std::string_view kDateIncorrectDayCount =
    "validation.date.incorrect_day_count";
inline constexpr std::string_view kDateContinuityMissing =
    "validation.date.continuity_missing";

// Time
inline constexpr std::string_view kTimeDiscontinuity =
    "validation.time.discontinuity";

// Sleep
inline constexpr std::string_view kSleepMissingNight =
    "validation.sleep.missing_night";

// JSON / activity counts
inline constexpr std::string_view kJsonTooFewActivities =
    "validation.json.too_few_activities";

// Activity duration
inline constexpr std::string_view kActivityZeroDuration =
    "validation.activity.zero_duration";
inline constexpr std::string_view kActivityDurationTooLong =
    "validation.activity.duration_too_long";

// Generic structural / line / logical / file access
inline constexpr std::string_view kIoFileAccess = "validation.io.file_access";
inline constexpr std::string_view kStructureInvalid =
    "validation.structure.invalid";
inline constexpr std::string_view kLineInvalid = "validation.line.invalid";
inline constexpr std::string_view kLogicInvalid = "validation.logic.invalid";

}  // namespace validation

// ---------------------------------------------------------------------------
// pipeline.*   Pipeline orchestration stage failures
// ---------------------------------------------------------------------------
namespace pipeline {

inline constexpr std::string_view kStepFailed = "pipeline.step.failed";

}  // namespace pipeline

// ---------------------------------------------------------------------------
// import_.*   Import / ingest transaction failures
// (Note: "import_" avoids conflict with C++ module "import" keyword)
// ---------------------------------------------------------------------------
namespace import_ {

inline constexpr std::string_view kTransactionFailed =
    "import.transaction.failed";

}  // namespace import_

// ---------------------------------------------------------------------------
// export_.*   Export / report write failures
// ---------------------------------------------------------------------------
namespace export_ {

inline constexpr std::string_view kWriteFailed = "export.write.failed";

}  // namespace export_

// ---------------------------------------------------------------------------
// runtime.*   Runtime environment / dependency errors
// ---------------------------------------------------------------------------
namespace runtime {

inline constexpr std::string_view kIoPathNotFound = "runtime.io.path_not_found";
inline constexpr std::string_view kDependencyMissing =
    "runtime.dependency.missing";

}  // namespace runtime

// ---------------------------------------------------------------------------
// io.*   Low-level file system read/write errors
// ---------------------------------------------------------------------------
namespace io {

inline constexpr std::string_view kReadFailed = "io.read.failed";
inline constexpr std::string_view kWriteFailed = "io.write.failed";

}  // namespace io

// ---------------------------------------------------------------------------
// config.*   Configuration load / parse errors
// ---------------------------------------------------------------------------
namespace config {

inline constexpr std::string_view kLoadFailed = "config.load.failed";
inline constexpr std::string_view kFieldInvalid = "config.field.invalid";

}  // namespace config

}  // namespace tracer_core::domain::errors::codes

#endif  // DOMAIN_ERRORS_ERROR_CODES_H_
