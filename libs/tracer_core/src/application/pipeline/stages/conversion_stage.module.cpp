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
import tracer.core.shared.ansi_colors;

using tracer::core::domain::modlogic::converter::LogProcessingResult;
using tracer::core::domain::modlogic::converter::LogProcessor;
namespace modcolors = tracer::core::shared::ansi_colors;

namespace tracer::core::application::pipeline {
namespace {

constexpr double kMillisPerSecond = 1000.0;

}  // namespace

auto ConversionStage::Execute(PipelineSession& session) -> bool {
  tracer_core::application::ports::LogInfo(
      "Step: Converting files (Parallel)...");
  const auto start_time = std::chrono::steady_clock::now();

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
            const std::string source_label = input.source_label.empty()
                                                 ? input.source_id
                                                 : input.source_label;
            tracer_core::application::ports::LogError(
                std::string(modcolors::kRed) +
                "Thread Error [" + source_label + "]: " + e.what() +
                std::string(modcolors::kReset));
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

  const auto end_time = std::chrono::steady_clock::now();
  const double duration =
      std::chrono::duration<double, std::milli>(end_time - start_time).count();
  PrintTiming(duration);

  if (all_success) {
    tracer_core::application::ports::LogInfo(
        std::string(modcolors::kGreen) +
        "内存转换阶段 全部成功 (" + std::to_string(processed_count) +
        " files)." + std::string(modcolors::kReset));
  } else {
    tracer_core::application::ports::LogWarn(
        std::string(modcolors::kYellow) +
        "内存转换阶段 完成，但存在部分错误。" +
        std::string(modcolors::kReset));
  }

  return all_success;
}

void ConversionStage::PrintTiming(double total_time_ms) {
  const double total_time_s = total_time_ms / kMillisPerSecond;

  std::ostringstream stream;
  stream << "--------------------------------------\n";
  stream << "转换耗时: " << std::fixed << std::setprecision(3) << total_time_s
         << " 秒 (" << total_time_ms << " ms)\n";
  stream << "--------------------------------------";

  tracer_core::application::ports::LogInfo(stream.str());
}

}  // namespace tracer::core::application::pipeline
