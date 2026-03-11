module;

#include "domain/ports/diagnostics.hpp"

export module tracer.core.domain.ports.diagnostics;

export namespace tracer::core::domain::ports {

#include "domain/detail/diagnostics_contract.inc"

}  // namespace tracer::core::domain::ports

export namespace tracer::core::domain::modports {

using tracer::core::domain::ports::AppendErrorReport;
using tracer::core::domain::ports::ClearBufferedDiagnostics;
using tracer::core::domain::ports::ClearDiagnosticsDedup;
using tracer::core::domain::ports::DiagnosticSeverity;
using tracer::core::domain::ports::EmitError;
using tracer::core::domain::ports::EmitInfo;
using tracer::core::domain::ports::EmitWarn;
using tracer::core::domain::ports::GetBufferedDiagnosticsSummary;
using tracer::core::domain::ports::GetCurrentRunErrorLogPath;
using tracer::core::domain::ports::GetDiagnosticsSink;
using tracer::core::domain::ports::GetErrorReportDestinationLabel;
using tracer::core::domain::ports::GetErrorReportWriter;
using tracer::core::domain::ports::IDiagnosticsSink;
using tracer::core::domain::ports::IErrorReportWriter;
using tracer::core::domain::ports::SetDiagnosticsSink;
using tracer::core::domain::ports::SetErrorReportWriter;

}  // namespace tracer::core::domain::modports
