module;

#include <chrono>
#include <future>
#include <iomanip>
#include <iterator>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

#include "application/ports/logger.hpp"

module tracer.core.application.pipeline.stages;

import tracer.core.application.pipeline.types;
import tracer.core.domain.logic.converter.log_processor;

using tracer::core::domain::modlogic::converter::LogProcessingResult;
using tracer::core::domain::modlogic::converter::LogProcessor;

namespace tracer::core::application::pipeline {
namespace {

constexpr double kMillisPerSecond = 1000.0;

}  // namespace

auto ConversionStage::Execute(PipelineSession& session) -> bool {
  if (session.config.structure_validation_blocks_conversion) {
    tracer_core::application::ports::LogInfo(
        "Step: Converting files after structure precheck (Parallel)...");
  } else {
    tracer_core::application::ports::LogInfo(
        "Step: Converting files (Parallel)...");
  }
  const auto kStartTime = std::chrono::steady_clock::now();

  std::mutex data_mutex;
  std::vector<std::future<LogProcessingResult>> futures;

  for (const auto& input : session.state.ingest_inputs) {
    futures.push_back(std::async(
        std::launch::async, [&session, input]() -> LogProcessingResult {
          try {
            LogProcessor processor(session.state.converter_config);
            return processor.ProcessSourceContent(input.source_id,
                                                  input.content);

          } catch (const std::exception& e) {
            const std::string kSourceLabel = input.source_label.empty()
                                                 ? input.source_id
                                                 : input.source_label;
            tracer_core::application::ports::LogError(
                "Thread Error [" + kSourceLabel + "]: " + e.what());
            return LogProcessingResult{.success = false, .processed_data = {}};
          }
        }));
  }

  bool all_success = true;
  int processed_count = 0;

  for (auto& future : futures) {
    LogProcessingResult result = future.get();
    ++processed_count;

    if (!result.success) {
      all_success = false;
      continue;
    }

    std::scoped_lock lock(data_mutex);
    for (auto& [year_month_key, month_days] : result.processed_data) {
      auto& merged_days = session.result.processed_data[year_month_key];
      merged_days.insert(merged_days.end(),
                         std::make_move_iterator(month_days.begin()),
                         std::make_move_iterator(month_days.end()));
    }
  }

  const auto kEndTime = std::chrono::steady_clock::now();
  const double kDuration =
      std::chrono::duration<double, std::milli>(kEndTime - kStartTime).count();
  PrintTiming(kDuration);

  if (all_success) {
    tracer_core::application::ports::LogInfo(
        "转换阶段 全部成功 (" + std::to_string(processed_count) +
        " files).");
  } else {
    tracer_core::application::ports::LogWarn(
        "转换阶段 完成，但存在部分错误。");
  }

  return all_success;
}

void ConversionStage::PrintTiming(double total_time_ms) {
  const double kTotalTimeS = total_time_ms / kMillisPerSecond;

  std::ostringstream stream;
  stream << "--------------------------------------\n";
  stream << "转换耗时: " << std::fixed << std::setprecision(3) << kTotalTimeS
         << " 秒 (" << total_time_ms << " ms)\n";
  stream << "--------------------------------------";

  tracer_core::application::ports::LogInfo(stream.str());
}

}  // namespace tracer::core::application::pipeline
