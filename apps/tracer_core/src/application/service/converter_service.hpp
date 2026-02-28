// application/service/converter_service.hpp
#ifndef APPLICATION_SERVICE_CONVERTER_SERVICE_H_
#define APPLICATION_SERVICE_CONVERTER_SERVICE_H_

#include <functional>
#include <iostream>
#include <string_view>

#include "domain/model/daily_log.hpp"
#include "domain/types/converter_config.hpp"

class ConverterService {
 public:
  explicit ConverterService(const ConverterConfig& config);

  auto ExecuteConversion(std::istream& combined_input_stream,
                         std::function<void(DailyLog&&)> data_consumer,
                         std::string_view source_file) -> void;

 private:
  const ConverterConfig& config_;

  static auto IsDuplicateAcrossYear(const DailyLog& previous_day,
                                    const DailyLog& current_day) -> bool;
};

#endif  // APPLICATION_SERVICE_CONVERTER_SERVICE_H_
