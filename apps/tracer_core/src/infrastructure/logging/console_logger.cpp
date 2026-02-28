// infrastructure/logging/console_logger.cpp
#include "infrastructure/logging/console_logger.hpp"

#include <iostream>

namespace infrastructure::logging {

auto ConsoleLogger::Log(tracer_core::application::ports::LogSeverity severity,
                        std::string_view message) -> void {
  std::scoped_lock lock(output_mutex_);
  if (severity == tracer_core::application::ports::LogSeverity::kError) {
    std::cerr << message << std::endl;
    return;
  }
  std::cout << message << std::endl;
}

}  // namespace infrastructure::logging
