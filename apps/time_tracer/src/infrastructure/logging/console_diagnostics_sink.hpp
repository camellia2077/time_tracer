// infrastructure/logging/console_diagnostics_sink.hpp
#ifndef INFRASTRUCTURE_LOGGING_CONSOLE_DIAGNOSTICS_SINK_H_
#define INFRASTRUCTURE_LOGGING_CONSOLE_DIAGNOSTICS_SINK_H_

#include "domain/ports/diagnostics.hpp"

namespace infrastructure::logging {

class ConsoleDiagnosticsSink final
    : public time_tracer::domain::ports::IDiagnosticsSink {
 public:
  auto Emit(time_tracer::domain::ports::DiagnosticSeverity severity,
            std::string_view message) -> void override;
};

}  // namespace infrastructure::logging

#endif  // INFRASTRUCTURE_LOGGING_CONSOLE_DIAGNOSTICS_SINK_H_
