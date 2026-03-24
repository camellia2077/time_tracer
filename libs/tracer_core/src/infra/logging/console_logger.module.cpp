module;

#include <iostream>
#include <mutex>

#include "application/runtime_bridge/logger.hpp"

module tracer.core.infrastructure.logging.console_logger;

namespace tracer::core::infrastructure::logging {

auto ConsoleLogger::Log(
    tracer_core::application::runtime_bridge::LogSeverity severity,
    std::string_view message) -> void {
  std::scoped_lock lock(output_mutex_);
  if (severity ==
      tracer_core::application::runtime_bridge::LogSeverity::kError) {
    std::cerr << message << std::endl;
    return;
  }
  std::cout << message << std::endl;
}

}  // namespace tracer::core::infrastructure::logging
