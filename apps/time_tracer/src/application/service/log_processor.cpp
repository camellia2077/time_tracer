// application/service/log_processor.cpp
#include "domain/logic/converter/log_processor.hpp"

#include <sstream>

#include "application/service/converter_service.hpp"
#include "domain/ports/diagnostics.hpp"

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
    time_tracer::domain::ports::EmitError(
        std::string("An error occurred during conversion: ") + e.what());
    result.success = false;
  } catch (...) {
    result.success = false;
  }

  return result;
}
