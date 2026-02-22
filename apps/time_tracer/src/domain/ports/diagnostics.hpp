// domain/ports/diagnostics.hpp
#ifndef DOMAIN_PORTS_DIAGNOSTICS_H_
#define DOMAIN_PORTS_DIAGNOSTICS_H_

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>

namespace time_tracer::domain::ports {

enum class DiagnosticSeverity {
  kInfo,
  kWarn,
  kError,
};

class IDiagnosticsSink {
 public:
  virtual ~IDiagnosticsSink() = default;
  virtual auto Emit(DiagnosticSeverity severity, std::string_view message)
      -> void = 0;
};

class IErrorReportWriter {
 public:
  virtual ~IErrorReportWriter() = default;
  virtual auto Append(std::string_view report_content) -> bool = 0;
  [[nodiscard]] virtual auto DestinationLabel() const -> std::string = 0;
};

auto SetDiagnosticsSink(std::shared_ptr<IDiagnosticsSink> sink) -> void;
auto GetDiagnosticsSink() -> std::shared_ptr<IDiagnosticsSink>;

auto SetErrorReportWriter(std::shared_ptr<IErrorReportWriter> writer) -> void;
auto GetErrorReportWriter() -> std::shared_ptr<IErrorReportWriter>;

auto EmitInfo(std::string_view message) -> void;
auto EmitWarn(std::string_view message) -> void;
auto EmitError(std::string_view message) -> void;

auto AppendErrorReport(std::string_view report_content) -> bool;
auto GetErrorReportDestinationLabel() -> std::string;

auto ClearBufferedDiagnostics() -> void;
auto GetBufferedDiagnosticsSummary(DiagnosticSeverity minimum_severity,
                                   std::size_t max_entries) -> std::string;

// Session-level dedup: clears the set of already-emitted messages.
// Call at the start of each run to reset deduplication state.
auto ClearDiagnosticsDedup() -> void;

// Returns the file path of the current run's error log.
// Delegates to the active IErrorReportWriter; returns "disabled" when none.
auto GetCurrentRunErrorLogPath() -> std::string;

}  // namespace time_tracer::domain::ports

#endif  // DOMAIN_PORTS_DIAGNOSTICS_H_
