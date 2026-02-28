// application/ports/i_validation_issue_reporter.hpp
#ifndef APPLICATION_PORTS_I_VALIDATION_ISSUE_REPORTER_H_
#define APPLICATION_PORTS_I_VALIDATION_ISSUE_REPORTER_H_

#include <set>
#include <string_view>
#include <vector>

#include "domain/logic/validator/common/diagnostic.hpp"
#include "domain/logic/validator/common/validator_utils.hpp"

namespace tracer_core::application::ports {

class IValidationIssueReporter {
 public:
  virtual ~IValidationIssueReporter() = default;

  virtual auto ReportStructureErrors(std::string_view display_label,
                                     const std::set<validator::Error>& errors)
      -> void = 0;

  virtual auto ReportLogicDiagnostics(
      std::string_view fallback_label,
      const std::vector<validator::Diagnostic>& diagnostics) -> void = 0;
};

}  // namespace tracer_core::application::ports

#endif  // APPLICATION_PORTS_I_VALIDATION_ISSUE_REPORTER_H_
