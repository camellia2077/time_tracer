#if TT_ENABLE_CPP20_MODULES
module;

#include <string>
#include <vector>

#include "application/ports/i_validation_issue_reporter.hpp"
#include "application/ports/logger.hpp"

module tracer.core.application.pipeline.stages;

import tracer.core.application.pipeline.types;
import tracer.core.domain.logic.validator.common.diagnostic;
import tracer.core.domain.logic.validator.structure;
import tracer.core.shared.ansi_colors;

using tracer::core::domain::modlogic::validator_common::Diagnostic;
using tracer::core::domain::modlogic::validator_structure::StructValidator;
namespace modcolors = tracer::core::shared::modcolors;

#else

#include "application/pipeline/pipeline_stages.hpp"

#include <string>
#include <vector>

#include "application/ports/i_validation_issue_reporter.hpp"
#include "application/ports/logger.hpp"
#include "domain/logic/validator/common/diagnostic.hpp"
#include "domain/logic/validator/structure/structure_validator.hpp"
#include "shared/types/ansi_colors.hpp"

using ::validator::Diagnostic;
using ::validator::structure::StructValidator;
namespace modcolors = tracer_core::common::colors;

#endif

namespace tracer::core::application::pipeline {
namespace {

auto ResolveLogicFallbackLabel(const std::string& month_key,
                               const std::vector<DailyLog>& days)
    -> std::string {
  for (const auto& day : days) {
    if (day.source_span.has_value() && !day.source_span->file_path.empty()) {
      return day.source_span->file_path;
    }
  }

  for (const auto& day : days) {
    for (const auto& activity : day.processedActivities) {
      if (activity.source_span.has_value() &&
          !activity.source_span->file_path.empty()) {
        return activity.source_span->file_path;
      }
    }
  }

  return "ProcessedData[" + month_key + "]";
}

}  // namespace

auto LogicValidationStage::Execute(PipelineSession& session) -> bool {
  tracer_core::application::ports::LogInfo(
      "Step: Validating Business Logic (Dates, Continuity)...");

  if (session.result.processed_data.empty()) {
    tracer_core::application::ports::LogWarn(
        std::string(modcolors::kYellow) + "No data to validate." +
        std::string(modcolors::kReset));
    return true;
  }

  StructValidator validator(session.config.date_check_mode);

  bool all_valid = true;
  for (const auto& [month_key, days] : session.result.processed_data) {
    if (days.empty()) {
      continue;
    }

    const std::string fallback_label =
        ResolveLogicFallbackLabel(month_key, days);

    std::vector<Diagnostic> diagnostics;
    if (!validator.Validate(fallback_label, days, diagnostics)) {
      all_valid = false;
      if (session.state.validation_issue_reporter != nullptr) {
        session.state.validation_issue_reporter->ReportLogicDiagnostics(
            fallback_label, diagnostics);
      }
    }
  }

  if (all_valid) {
    tracer_core::application::ports::LogInfo(
        std::string(modcolors::kGreen) + "Logic validation passed." +
        std::string(modcolors::kReset));
  } else {
    tracer_core::application::ports::LogError(
        std::string(modcolors::kRed) +
        "Logic validation found issues (e.g., broken date continuity)." +
        std::string(modcolors::kReset));
  }

  return all_valid;
}

}  // namespace tracer::core::application::pipeline
