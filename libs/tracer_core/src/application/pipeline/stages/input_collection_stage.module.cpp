module;

#include <string>

#include "application/ports/i_ingest_input_provider.hpp"
#include "application/ports/logger.hpp"

module tracer.core.application.pipeline.stages;

import tracer.core.application.pipeline.types;
import tracer.core.shared.ansi_colors;

namespace modcolors = tracer::core::shared::modcolors;

namespace tracer::core::application::pipeline {

auto InputCollectionStage::Execute(
    PipelineSession& session,
    const tracer_core::application::ports::IIngestInputProvider& input_provider,
    const std::string& extension) -> bool {
  session.state.ingest_inputs.clear();
  session.state.generated_files.clear();

  const auto input_collection =
      input_provider.CollectTextInputs(session.config.input_root, extension);

  if (!input_collection.input_exists) {
    tracer_core::application::ports::LogError(
        std::string(modcolors::kRed) +
        "错误: 输入的路径不存在: " + session.config.input_root.string() +
        std::string(modcolors::kReset));
    return false;
  }

  if (input_collection.inputs.empty()) {
    tracer_core::application::ports::LogWarn(
        std::string(modcolors::kYellow) +
        "警告: 在指定路径下没有找到 " + extension + " 文件。" +
        std::string(modcolors::kReset));
    return false;
  }

  session.state.ingest_inputs = input_collection.inputs;
  tracer_core::application::ports::LogInfo(
      "信息: 成功收集到 " +
      std::to_string(session.state.ingest_inputs.size()) +
      " 个待处理文件 (" + extension + ").");
  return true;
}

}  // namespace tracer::core::application::pipeline
