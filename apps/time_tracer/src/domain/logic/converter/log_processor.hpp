// domain/logic/converter/log_processor.hpp
#ifndef CONVERTER_LOG_PROCESSOR_H_
#define CONVERTER_LOG_PROCESSOR_H_

#include <functional>
#include <istream>
#include <map>
#include <string>
#include <string_view>
#include <vector>

#include "domain/model/daily_log.hpp"
// [重构] 引用 Common 定义的配置结构体
#include "infrastructure/config/models/converter_config_models.hpp"

struct LogProcessingResult {
  bool success = true;
  std::map<std::string, std::vector<DailyLog>> processed_data;
};

class LogProcessor {
 public:
  explicit LogProcessor(const ConverterConfig& config);

  void ConvertStreamToData(std::istream& combined_stream,
                           std::function<void(DailyLog&&)> data_consumer,
                           std::string_view source_file);

  auto ProcessSourceContent(
      const std::string& filename,
      const std::string&
          content)  // NOLINT(bugprone-easily-swappable-parameters)
      -> LogProcessingResult;

 private:
  const ConverterConfig& config_;
};

#endif  // CONVERTER_LOG_PROCESSOR_H_
