// infrastructure/logging/console_diagnostics_sink.hpp
#ifndef INFRASTRUCTURE_LOGGING_CONSOLE_DIAGNOSTICS_SINK_H_
#define INFRASTRUCTURE_LOGGING_CONSOLE_DIAGNOSTICS_SINK_H_

#include "domain/ports/diagnostics.hpp"

namespace tracer::core::infrastructure::logging {

#include "infrastructure/logging/detail/console_diagnostics_sink_decl.inc"

}  // namespace tracer::core::infrastructure::logging

namespace infrastructure::logging {

using tracer::core::infrastructure::logging::ConsoleDiagnosticsSink;

}  // namespace infrastructure::logging

#endif  // INFRASTRUCTURE_LOGGING_CONSOLE_DIAGNOSTICS_SINK_H_
