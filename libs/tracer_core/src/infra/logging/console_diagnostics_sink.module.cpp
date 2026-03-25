module;

#include <iostream>

module tracer.core.infrastructure.logging.console_diagnostics_sink;

import tracer.core.domain.ports.diagnostics;

namespace tracer::core::infrastructure::logging {

namespace modports = tracer::core::domain::ports;

auto ConsoleDiagnosticsSink::Emit(const modports::DiagnosticSeverity kSeverity,
                                  const std::string_view kMessage) -> void {
  if (kSeverity == modports::DiagnosticSeverity::kError) {
    std::cerr << kMessage << std::endl;
    return;
  }
  std::cout << kMessage << std::endl;
}

}  // namespace tracer::core::infrastructure::logging
