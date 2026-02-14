// infrastructure/logging/console_diagnostics_sink.cpp
#include "infrastructure/logging/console_diagnostics_sink.hpp"

#include <iostream>

namespace infrastructure::logging {

auto ConsoleDiagnosticsSink::Emit(
    const time_tracer::domain::ports::DiagnosticSeverity kSeverity,
    const std::string_view kMessage) -> void {
  if (kSeverity == time_tracer::domain::ports::DiagnosticSeverity::kError) {
    std::cerr << kMessage << std::endl;
    return;
  }
  std::cout << kMessage << std::endl;
}

}  // namespace infrastructure::logging
