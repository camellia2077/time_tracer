// application/service/log_processor.cpp
import tracer.core.application.service.converter;
import tracer.core.domain.logic.converter.log_processor;
import tracer.core.domain.model.daily_log;
import tracer.core.domain.ports.diagnostics;
import tracer.core.domain.types.converter_config;

#include <functional>
#include <istream>
#include <sstream>
#include <string>
#include <string_view>

using tracer::core::application::modservice::ConverterService;
using tracer::core::domain::modlogic::converter::LogProcessingResult;
using tracer::core::domain::modlogic::converter::LogProcessor;
using tracer::core::domain::modmodel::DailyLog;
using tracer::core::domain::modtypes::ConverterConfig;

namespace modports = tracer::core::domain::ports;

LogProcessor::LogProcessor(const ConverterConfig& config) : config_(config) {}

void LogProcessor::ConvertStreamToData(
    std::istream& combined_stream,
    std::function<void(DailyLog&&)> data_consumer,
    std::string_view source_file) {
  ConverterService processor(config_);
  processor.ExecuteConversion(combined_stream, data_consumer, source_file);
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
auto LogProcessor::ProcessSourceContent(const std::string& filename,
                                        const std::string& content)
    -> LogProcessingResult {
  LogProcessingResult result;
  result.success = true;

  try {
    std::stringstream string_stream(content);
    ConvertStreamToData(
        string_stream,
        [&](DailyLog&& log) -> void {
          constexpr size_t kYearMonthLen = 7;
          std::string key = log.date.substr(0, kYearMonthLen);  // YYYY-MM
          result.processed_data[key].push_back(std::move(log));
        },
        filename);
  } catch (const std::exception& e) {
    modports::EmitError(std::string("An error occurred during conversion: ") +
                        e.what());
    result.success = false;
  } catch (...) {
    result.success = false;
  }

  return result;
}
