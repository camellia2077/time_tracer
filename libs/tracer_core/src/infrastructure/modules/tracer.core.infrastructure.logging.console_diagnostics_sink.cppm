module;

#include "domain/ports/diagnostics.hpp"

export module tracer.core.infrastructure.logging.console_diagnostics_sink;

export namespace tracer::core::infrastructure::logging {

#include "infrastructure/logging/detail/console_diagnostics_sink_decl.inc"

}  // namespace tracer::core::infrastructure::logging

export namespace tracer::core::infrastructure::modlogging {

using tracer::core::infrastructure::logging::ConsoleDiagnosticsSink;

}  // namespace tracer::core::infrastructure::modlogging
