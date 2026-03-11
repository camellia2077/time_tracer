// infrastructure/logging/validation_issue_reporter.hpp
#ifndef INFRASTRUCTURE_LOGGING_VALIDATION_ISSUE_REPORTER_H_
#define INFRASTRUCTURE_LOGGING_VALIDATION_ISSUE_REPORTER_H_

#include "application/ports/i_validation_issue_reporter.hpp"

namespace tracer::core::infrastructure::logging {

#include "infrastructure/logging/detail/validation_issue_reporter_decl.inc"

}  // namespace tracer::core::infrastructure::logging

namespace infrastructure::logging {

using tracer::core::infrastructure::logging::ValidationIssueReporter;

}  // namespace infrastructure::logging

#endif  // INFRASTRUCTURE_LOGGING_VALIDATION_ISSUE_REPORTER_H_
