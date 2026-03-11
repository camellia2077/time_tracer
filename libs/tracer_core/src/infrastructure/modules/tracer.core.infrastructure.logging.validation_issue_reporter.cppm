module;

#include "application/ports/i_validation_issue_reporter.hpp"

export module tracer.core.infrastructure.logging.validation_issue_reporter;

export namespace tracer::core::infrastructure::logging {

#include "infrastructure/logging/detail/validation_issue_reporter_decl.inc"

}  // namespace tracer::core::infrastructure::logging

export namespace tracer::core::infrastructure::modlogging {

using tracer::core::infrastructure::logging::ValidationIssueReporter;

}  // namespace tracer::core::infrastructure::modlogging
