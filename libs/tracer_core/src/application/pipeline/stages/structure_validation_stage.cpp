#if TT_ENABLE_CPP20_MODULES
module;

#include <set>
#include <string>

#include "application/ports/i_validation_issue_reporter.hpp"
#include "application/ports/logger.hpp"

module tracer.core.application.pipeline.stages;

import tracer.core.application.pipeline.types;
import tracer.core.domain.logic.validator.common.validator_utils;
import tracer.core.domain.logic.validator.txt.facade;
import tracer.core.shared.ansi_colors;

using tracer::core::domain::modlogic::validator_common::Error;
using tracer::core::domain::modlogic::validator_txt::TextValidator;

#else

#include "application/pipeline/pipeline_stages.hpp"

#include <set>
#include <string>

#include "application/ports/i_validation_issue_reporter.hpp"
#include "application/ports/logger.hpp"
#include "domain/logic/validator/common/validator_utils.hpp"
#include "domain/logic/validator/txt/facade/text_validator.hpp"
#include "shared/types/ansi_colors.hpp"

using ::validator::Error;
using ::validator::txt::TextValidator;

#endif

namespace modcolors = tracer::core::shared::ansi_colors;

namespace tracer::core::application::pipeline {

auto StructureValidationStage::Execute(PipelineSession& session) -> bool {
  tracer_core::application::ports::LogInfo(
      "Step: Validating Source Structure (TXT)...");

  TextValidator validator(session.state.converter_config);

  bool all_valid = true;
  int files_checked = 0;

  for (const auto& input : session.state.ingest_inputs) {
    ++files_checked;
    const std::string source_path =
        input.source_id.empty() ? input.source_label : input.source_id;
    const std::string display_label =
        input.source_label.empty() ? source_path : input.source_label;

    std::set<Error> errors;
    if (!validator.Validate(source_path, input.content, errors)) {
      all_valid = false;
      if (session.state.validation_issue_reporter != nullptr) {
        session.state.validation_issue_reporter->ReportStructureErrors(
            display_label, errors);
      }
    }
  }

  if (all_valid) {
    tracer_core::application::ports::LogInfo(
        std::string(modcolors::kGreen) +
        "Structure validation passed for " + std::to_string(files_checked) +
        " files." + std::string(modcolors::kReset));
  } else {
    tracer_core::application::ports::LogError(
        std::string(modcolors::kRed) +
        "Structure validation failed. Please fix the errors above." +
        std::string(modcolors::kReset));
  }

  return all_valid;
}

}  // namespace tracer::core::application::pipeline
