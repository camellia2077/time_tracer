// infrastructure/logging/console_logger.hpp
#ifndef INFRASTRUCTURE_LOGGING_CONSOLE_LOGGER_H_
#define INFRASTRUCTURE_LOGGING_CONSOLE_LOGGER_H_

#include <mutex>

#include "application/ports/logger.hpp"

namespace tracer::core::infrastructure::logging {

#include "infrastructure/logging/detail/console_logger_decl.inc"

}  // namespace tracer::core::infrastructure::logging

namespace infrastructure::logging {

using tracer::core::infrastructure::logging::ConsoleLogger;

}  // namespace infrastructure::logging

#endif  // INFRASTRUCTURE_LOGGING_CONSOLE_LOGGER_H_
