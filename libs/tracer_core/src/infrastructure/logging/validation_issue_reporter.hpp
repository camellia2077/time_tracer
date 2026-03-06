// infrastructure/logging/validation_issue_reporter.hpp
#ifndef INFRASTRUCTURE_LOGGING_VALIDATION_ISSUE_REPORTER_H_
#define INFRASTRUCTURE_LOGGING_VALIDATION_ISSUE_REPORTER_H_

#include "application/ports/i_validation_issue_reporter.hpp"

namespace infrastructure::logging {

class ValidationIssueReporter final
    : public tracer_core::application::ports::IValidationIssueReporter {
 public:
  auto ReportStructureErrors(std::string_view display_label,
                             const std::set<validator::Error>& errors)
      -> void override;

  auto ReportLogicDiagnostics(
      std::string_view fallback_label,
      const std::vector<validator::Diagnostic>& diagnostics) -> void override;
};

}  // namespace infrastructure::logging

#endif  // INFRASTRUCTURE_LOGGING_VALIDATION_ISSUE_REPORTER_H_
