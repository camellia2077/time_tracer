module;

#include "domain/ports/diagnostics.hpp"

export module tracer.core.infrastructure.logging.console_diagnostics_sink;

export namespace tracer::core::infrastructure::logging {

class ConsoleDiagnosticsSink final
    : public tracer_core::domain::ports::IDiagnosticsSink {
 public:
  auto Emit(tracer_core::domain::ports::DiagnosticSeverity severity,
            std::string_view message) -> void override;
};

}  // namespace tracer::core::infrastructure::logging
