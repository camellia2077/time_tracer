module;

#include <string>

#include "application/ports/logger.hpp"

module tracer.core.application.pipeline.stages;

import tracer.core.application.pipeline.types;
import tracer.core.domain.logic.converter.core;
import tracer.core.shared.ansi_colors;

using tracer::core::domain::modlogic::converter::LogLinker;
namespace modcolors = tracer::core::shared::modcolors;

namespace tracer::core::application::pipeline {

auto CrossMonthLinkStage::Execute(PipelineSession& session) -> bool {
  if (session.result.processed_data.empty()) {
    return true;
  }

  tracer_core::application::ports::LogInfo(
      "Step: Linking cross-month data...");

  try {
    LogLinker linker(session.state.converter_config);
    linker.LinkLogs(session.result.processed_data);

  } catch (const std::exception& e) {
    tracer_core::application::ports::LogError(
        std::string(modcolors::kRed) +
        "[Pipeline] Logic Linker Error: " + e.what() +
        std::string(modcolors::kReset));
    return true;
  }

  return true;
}

}  // namespace tracer::core::application::pipeline
