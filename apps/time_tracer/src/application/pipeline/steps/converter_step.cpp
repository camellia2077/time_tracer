// application/pipeline/steps/converter_step.cpp
#include "application/pipeline/steps/converter_step.hpp"

#include <chrono>
#include <future>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <vector>

#include "common/ansi_colors.hpp"
#include "converter/log_processor.hpp"
#include "infrastructure/io/core/file_reader.hpp"

namespace core::pipeline {

namespace {
constexpr double kMillisPerSecond = 1000.0;
}  // namespace

ConverterStep::ConverterStep(const AppConfig& /*unused*/) {}

auto ConverterStep::Execute(PipelineContext& context) -> bool {

  std::cout << "Step: Converting files (Parallel)..." << std::endl;
  auto start_time = std::chrono::steady_clock::now();

  std::mutex data_mutex;
  std::vector<std::future<LogProcessingResult>> futures;

  // 1. 启动并行任务
  for (const auto& file_path : context.state.source_files) {
    // [注意] context.state.converter_config 现在是 Struct
    // 通过值传递或引用传递给 lambda 都是安全的，因为它是只读配置
    futures.push_back(std::async(
        std::launch::async, [&context, file_path]() -> LogProcessingResult {
          try {
            std::string content = FileReader::ReadContent(file_path);

            // LogProcessor 构造函数接收 const ConverterConfig& (Struct)
            LogProcessor processor(context.state.converter_config);
            return processor.ProcessSourceContent(file_path.string(), content);


          } catch (const std::exception& e) {
            std::cerr << time_tracer::common::colors::kRed << "Thread Error [" << file_path.filename()
                      << "]: " << e.what() << time_tracer::common::colors::kReset << std::endl;
            return LogProcessingResult{.success = false, .processed_data = {}};
          }
        }));
  }

  // 2. 收集结果
  bool all_success = true;
  int processed_count = 0;

  for (auto& future : futures) {
    LogProcessingResult result = future.get();
    processed_count++;

    if (!result.success) {
      all_success = false;
    } else {
      std::scoped_lock lock(data_mutex);
      context.result.processed_data.insert(result.processed_data.begin(),
                                           result.processed_data.end());
    }
  }

  auto end_time = std::chrono::steady_clock::now();
  double duration =
      std::chrono::duration<double, std::milli>(end_time - start_time).count();
  PrintTiming(duration);


  if (all_success) {
    std::cout << time_tracer::common::colors::kGreen << "内存转换阶段 全部成功 (" << processed_count
              << " files)." << time_tracer::common::colors::kReset << std::endl;
  } else {
    std::cout << time_tracer::common::colors::kYellow << "内存转换阶段 完成，但存在部分错误。"
              << time_tracer::common::colors::kReset << std::endl;
  }

  return all_success;
}

void ConverterStep::PrintTiming(double total_time_ms) {

  double total_time_s = total_time_ms / kMillisPerSecond;
  std::cout << "--------------------------------------\n";
  std::cout << "转换耗时: " << std::fixed << std::setprecision(3)
            << total_time_s << " 秒 (" << total_time_ms << " ms)\n";
  std::cout << "--------------------------------------\n";
}

}  // namespace core::pipeline
