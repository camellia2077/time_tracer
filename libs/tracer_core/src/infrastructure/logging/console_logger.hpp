// infrastructure/logging/console_logger.hpp
#ifndef INFRASTRUCTURE_LOGGING_CONSOLE_LOGGER_H_
#define INFRASTRUCTURE_LOGGING_CONSOLE_LOGGER_H_

#include <mutex>

#include "application/ports/logger.hpp"

namespace infrastructure::logging {

class ConsoleLogger final : public tracer_core::application::ports::ILogger {
 public:
  auto Log(tracer_core::application::ports::LogSeverity severity,
           std::string_view message) -> void override;

 private:
  std::mutex output_mutex_;
};

}  // namespace infrastructure::logging

#endif  // INFRASTRUCTURE_LOGGING_CONSOLE_LOGGER_H_
