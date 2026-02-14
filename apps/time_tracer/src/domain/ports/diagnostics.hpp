// domain/ports/diagnostics.hpp
#ifndef DOMAIN_PORTS_DIAGNOSTICS_H_
#define DOMAIN_PORTS_DIAGNOSTICS_H_

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

}  // namespace time_tracer::domain::ports

#endif  // DOMAIN_PORTS_DIAGNOSTICS_H_
