// converter/convert/facade/converter_service.hpp
#ifndef CONVERTER_CONVERT_FACADE_CONVERTER_SERVICE_H_
#define CONVERTER_CONVERT_FACADE_CONVERTER_SERVICE_H_

// [重构] 引用 Common 定义的配置结构体
#include <functional>
#include <istream>
#include <string>

#include "common/config/models/converter_config_models.hpp"
#include "domain/model/daily_log.hpp"

/**
 * @brief 核心转换调度器 (Facade)
 * 负责协调 Parser 和 Processor，将输入流转换为 DailyLog 对象。
 */
class ConverterService {
 public:
  explicit ConverterService(const ConverterConfig& config);

  auto ExecuteConversion(std::istream& combined_input_stream,
                         std::function<void(DailyLog&&)> data_consumer) -> void;

 private:
  [[nodiscard]] static auto IsDuplicateAcrossYear(const DailyLog& previous_day,
                                                  const DailyLog& current_day)
      -> bool;

  const ConverterConfig& config_;
};

#endif  // CONVERTER_CONVERT_FACADE_CONVERTER_SERVICE_H_
