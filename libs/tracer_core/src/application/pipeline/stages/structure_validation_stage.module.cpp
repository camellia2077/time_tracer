module;

#include <set>
#include <string>

#include "application/ports/i_validation_issue_reporter.hpp"
#include "application/ports/logger.hpp"

module tracer.core.application.pipeline.stages;

import tracer.core.application.pipeline.types;
import tracer.core.domain.logic.validator.common.validator_utils;
import tracer.core.domain.logic.validator.txt.facade;

using tracer::core::domain::modlogic::validator_common::Error;
using tracer::core::domain::modlogic::validator_txt::TextValidator;

namespace tracer::core::application::pipeline {

auto StructureValidationStage::Execute(PipelineSession& session) -> bool {
  TextValidator validator(session.state.converter_config);

  bool all_valid = true;
  int files_checked = 0;

  for (const auto& input : session.state.ingest_inputs) {
    ++files_checked;
    const std::string kSourcePath =
        input.source_id.empty() ? input.source_label : input.source_id;
    const std::string kDisplayLabel =
        input.source_label.empty() ? kSourcePath : input.source_label;

    std::set<Error> errors;
    if (!validator.Validate(kSourcePath, input.content, errors)) {
      all_valid = false;
      if (session.state.validation_issue_reporter != nullptr) {
        session.state.validation_issue_reporter->ReportStructureErrors(
            kDisplayLabel, errors);
      }
    }
  }

  if (all_valid) {
    if (session.config.structure_validation_blocks_conversion) {
      tracer_core::application::ports::LogInfo(
          "Source structure precheck passed for " +
          std::to_string(files_checked) + " files. Proceeding to conversion.");
    } else {
      tracer_core::application::ports::LogInfo(
          "Structure validation passed for " + std::to_string(files_checked) +
          " files.");
    }
  } else {
    if (session.config.structure_validation_blocks_conversion) {
      tracer_core::application::ports::LogError(
          "Source structure precheck failed. Fix the errors above before "
          "conversion.");
    } else {
      tracer_core::application::ports::LogError(
          "Structure validation failed. Please fix the errors above.");
    }
  }

  return all_valid;
}

}  // namespace tracer::core::application::pipeline
