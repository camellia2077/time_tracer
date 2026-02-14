// domain/ports/diagnostics.cpp
#include "domain/ports/diagnostics.hpp"

#include <mutex>
#include <utility>

namespace time_tracer::domain::ports {
namespace {

class NullDiagnosticsSink final : public IDiagnosticsSink {
 public:
  auto Emit(DiagnosticSeverity /*severity*/, std::string_view /*message*/)
      -> void override {}
};

class NullErrorReportWriter final : public IErrorReportWriter {
 public:
  auto Append(std::string_view /*report_content*/) -> bool override {
    return false;
  }

  [[nodiscard]] auto DestinationLabel() const -> std::string override {
    return "disabled";
  }
};

std::mutex g_mutex;
std::shared_ptr<IDiagnosticsSink> g_diagnostics_sink =
    std::make_shared<NullDiagnosticsSink>();
std::shared_ptr<IErrorReportWriter> g_error_report_writer =
    std::make_shared<NullErrorReportWriter>();

}  // namespace

auto SetDiagnosticsSink(std::shared_ptr<IDiagnosticsSink> sink) -> void {
  std::scoped_lock lock(g_mutex);
  if (sink) {
    g_diagnostics_sink = std::move(sink);
  } else {
    g_diagnostics_sink = std::make_shared<NullDiagnosticsSink>();
  }
}

auto GetDiagnosticsSink() -> std::shared_ptr<IDiagnosticsSink> {
  std::scoped_lock lock(g_mutex);
  return g_diagnostics_sink;
}

auto SetErrorReportWriter(std::shared_ptr<IErrorReportWriter> writer) -> void {
  std::scoped_lock lock(g_mutex);
  if (writer) {
    g_error_report_writer = std::move(writer);
  } else {
    g_error_report_writer = std::make_shared<NullErrorReportWriter>();
  }
}

auto GetErrorReportWriter() -> std::shared_ptr<IErrorReportWriter> {
  std::scoped_lock lock(g_mutex);
  return g_error_report_writer;
}

auto EmitInfo(std::string_view message) -> void {
  GetDiagnosticsSink()->Emit(DiagnosticSeverity::kInfo, message);
}

auto EmitWarn(std::string_view message) -> void {
  GetDiagnosticsSink()->Emit(DiagnosticSeverity::kWarn, message);
}

auto EmitError(std::string_view message) -> void {
  GetDiagnosticsSink()->Emit(DiagnosticSeverity::kError, message);
}

auto AppendErrorReport(std::string_view report_content) -> bool {
  return GetErrorReportWriter()->Append(report_content);
}

auto GetErrorReportDestinationLabel() -> std::string {
  return GetErrorReportWriter()->DestinationLabel();
}

}  // namespace time_tracer::domain::ports
