// converter/log_processor.cpp
#include "converter/log_processor.hpp"

#include <iostream>
#include <sstream>

#include "common/ansi_colors.hpp"
#include "converter/convert/facade/converter_service.hpp"

LogProcessor::LogProcessor(const ConverterConfig& config) : config_(config) {}

void LogProcessor::ConvertStreamToData(
    std::istream& combined_stream,
    std::function<void(DailyLog&&)> data_consumer) {
  ConverterService processor(config_);
  processor.ExecuteConversion(combined_stream, data_consumer);
}


auto LogProcessor::ProcessSourceContent(const std::string& /*filename*/,
                                        const std::string& content)
    -> LogProcessingResult {

  LogProcessingResult result;
  result.success = true;

  // [核心修改] 移除内部验证逻辑
  // 现在的 LogProcessor 假设输入内容已经被 Validator 模块（或 Pipeline
  // 前置步骤）验证过了。 它只专注于 "Content -> Struct" 的转换。

  try {
    std::stringstream string_stream(content);
    // 使用 lambda 捕获 result 并填充数据
    ConvertStreamToData(string_stream, [&](DailyLog&& log) -> void {


      constexpr size_t kYearMonthLen = 7;
      std::string key = log.date.substr(0, kYearMonthLen);  // YYYY-MM
      result.processed_data[key].push_back(std::move(log));
    });
  } catch (const std::exception& e) {
    namespace colors = time_tracer::common::colors;
    std::cerr << colors::kRed
              << "An error occurred during conversion: " << e.what()
              << colors::kReset << std::endl;
    result.success = false;
  } catch (...) {
    result.success = false;
  }

  return result;
}
