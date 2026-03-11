module;

#include <mutex>

#include "application/ports/logger.hpp"

export module tracer.core.infrastructure.logging.console_logger;

export namespace tracer::core::infrastructure::logging {

#include "infrastructure/logging/detail/console_logger_decl.inc"

}  // namespace tracer::core::infrastructure::logging

export namespace tracer::core::infrastructure::modlogging {

using tracer::core::infrastructure::logging::ConsoleLogger;
using ::tracer_core::application::ports::LogSeverity;

}  // namespace tracer::core::infrastructure::modlogging
