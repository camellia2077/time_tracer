// application/pipeline/steps/converter_step.cpp
#include "converter_step.hpp"

#include <chrono>
#include <future>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <vector>

#include "common/ansi_colors.hpp"
#include "converter/log_processor.hpp"
#include "io/core/file_reader.hpp"

namespace core::pipeline {

ConverterStep::ConverterStep(const AppConfig& /*unused*/) {}

auto ConverterStep::execute(PipelineContext& context) -> bool {
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
            std::string content = FileReader::read_content(file_path);

            // LogProcessor 构造函数接收 const ConverterConfig& (Struct)
            LogProcessor processor(context.state.converter_config);
            return processor.processSourceContent(file_path.string(), content);

          } catch (const std::exception& e) {
            std::cerr << RED_COLOR << "Thread Error [" << file_path.filename()
                      << "]: " << e.what() << RESET_COLOR << std::endl;
            return LogProcessingResult{.success = false, .processed_data = {}};
          }
        }));
  }

  // 2. 收集结果
  bool all_success = true;
  int processed_count = 0;

  for (auto& f : futures) {
    LogProcessingResult result = f.get();
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
  printTiming(duration);

  if (all_success) {
    std::cout << GREEN_COLOR << "内存转换阶段 全部成功 (" << processed_count
              << " files)." << RESET_COLOR << std::endl;
  } else {
    std::cout << YELLOW_COLOR << "内存转换阶段 完成，但存在部分错误。"
              << RESET_COLOR << std::endl;
  }

  return all_success;
}

void ConverterStep::printTiming(double total_time_ms) {
  double total_time_s = total_time_ms / 1000.0;
  std::cout << "--------------------------------------\n";
  std::cout << "转换耗时: " << std::fixed << std::setprecision(3)
            << total_time_s << " 秒 (" << total_time_ms << " ms)\n";
  std::cout << "--------------------------------------\n";
}

}  // namespace core::pipeline
