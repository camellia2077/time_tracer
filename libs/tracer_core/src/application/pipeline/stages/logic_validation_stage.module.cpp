module;

#include <string>
#include <vector>

#include "application/ports/i_validation_issue_reporter.hpp"
#include "application/ports/logger.hpp"
#include "domain/model/daily_log.hpp"

module tracer.core.application.pipeline.stages;

import tracer.core.application.pipeline.types;
import tracer.core.domain.logic.validator.common.diagnostic;
import tracer.core.domain.logic.validator.structure;

using tracer::core::domain::modlogic::validator_common::Diagnostic;
using tracer::core::domain::modlogic::validator_structure::StructValidator;

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
    tracer_core::application::ports::LogWarn("No data to validate.");
    return true;
  }

  StructValidator validator(session.config.date_check_mode);

  bool all_valid = true;
  for (const auto& [month_key, days] : session.result.processed_data) {
    if (days.empty()) {
      continue;
    }

    const std::string kFallbackLabel =
        ResolveLogicFallbackLabel(month_key, days);

    std::vector<Diagnostic> diagnostics;
    if (!validator.Validate(kFallbackLabel, days, diagnostics)) {
      all_valid = false;
      if (session.state.validation_issue_reporter != nullptr) {
        session.state.validation_issue_reporter->ReportLogicDiagnostics(
            kFallbackLabel, diagnostics);
      }
    }
  }

  if (all_valid) {
    tracer_core::application::ports::LogInfo("Logic validation passed.");
  } else {
    tracer_core::application::ports::LogError(
        "Logic validation found issues (e.g., broken date continuity).");
  }

  return all_valid;
}

}  // namespace tracer::core::application::pipeline
