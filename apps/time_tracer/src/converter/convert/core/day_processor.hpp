// converter/convert/core/day_processor.hpp
#ifndef CONVERTER_CONVERT_CORE_DAY_PROCESSOR_H_
#define CONVERTER_CONVERT_CORE_DAY_PROCESSOR_H_

#include "domain/model/daily_log.hpp"
// [重构] 引用 Common 定义的配置结构体
#include "common/config/models/converter_config_models.hpp"

class DayProcessor {
 public:
  explicit DayProcessor(const ConverterConfig& config);
  void Process(DailyLog& previousDay, DailyLog& dayToProcess);

 private:
  const ConverterConfig& config_;
};

#endif  // CONVERTER_CONVERT_CORE_DAY_PROCESSOR_H_
