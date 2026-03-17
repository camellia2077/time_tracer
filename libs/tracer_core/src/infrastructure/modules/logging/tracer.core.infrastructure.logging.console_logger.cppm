module;

#include <mutex>

#include "application/ports/logger.hpp"

export module tracer.core.infrastructure.logging.console_logger;

export namespace tracer::core::infrastructure::logging {

class ConsoleLogger final : public tracer_core::application::ports::ILogger {
 public:
  auto Log(tracer_core::application::ports::LogSeverity severity,
           std::string_view message) -> void override;

 private:
  std::mutex output_mutex_;
};

}  // namespace tracer::core::infrastructure::logging
