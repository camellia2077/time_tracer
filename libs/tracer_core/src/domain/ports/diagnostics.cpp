// domain/ports/diagnostics.cpp
#include "domain/ports/diagnostics.hpp"

#include <algorithm>
#include <deque>
#include <mutex>
#include <ranges>
#include <sstream>
#include <unordered_set>
#include <utility>
#include <vector>

namespace tracer_core::domain::ports {
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
constexpr std::size_t kBufferedDiagnosticsCapacity = 256;

struct BufferedDiagnosticEntry {
  DiagnosticSeverity severity;
  std::string message;
};

std::deque<BufferedDiagnosticEntry> g_buffered_diagnostics;

// Session-level dedup set: gates Emit calls only (not file writes).
// All severities share one set; reset via ClearDiagnosticsDedup().
std::unordered_set<std::string> g_dedup_set;

auto ToSeverityRank(DiagnosticSeverity severity) -> int {
  switch (severity) {
    case DiagnosticSeverity::kInfo:
      return 0;
    case DiagnosticSeverity::kWarn:
      return 1;
    case DiagnosticSeverity::kError:
      return 2;
  }
  return 0;
}

auto ToSeverityLabel(DiagnosticSeverity severity) -> std::string_view {
  switch (severity) {
    case DiagnosticSeverity::kInfo:
      return "INFO";
    case DiagnosticSeverity::kWarn:
      return "WARN";
    case DiagnosticSeverity::kError:
      return "ERROR";
  }
  return "INFO";
}

void BufferDiagnosticLocked(DiagnosticSeverity severity,
                            std::string_view message) {
  if (message.empty()) {
    return;
  }
  g_buffered_diagnostics.push_back(
      {.severity = severity, .message = std::string(message)});
  while (g_buffered_diagnostics.size() > kBufferedDiagnosticsCapacity) {
    g_buffered_diagnostics.pop_front();
  }
}

void EmitAndBuffer(DiagnosticSeverity severity, std::string_view message) {
  std::shared_ptr<IDiagnosticsSink> sink;
  bool already_emitted = false;
  {
    std::scoped_lock lock(g_mutex);
    sink = g_diagnostics_sink;
    BufferDiagnosticLocked(severity, message);
    already_emitted = !g_dedup_set.emplace(message).second;
  }
  if (!already_emitted) {
    sink->Emit(severity, message);
  }
}

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
  EmitAndBuffer(DiagnosticSeverity::kInfo, message);
}

auto EmitWarn(std::string_view message) -> void {
  EmitAndBuffer(DiagnosticSeverity::kWarn, message);
}

auto EmitError(std::string_view message) -> void {
  EmitAndBuffer(DiagnosticSeverity::kError, message);
}

auto AppendErrorReport(std::string_view report_content) -> bool {
  return GetErrorReportWriter()->Append(report_content);
}

auto GetErrorReportDestinationLabel() -> std::string {
  return GetErrorReportWriter()->DestinationLabel();
}

auto ClearBufferedDiagnostics() -> void {
  std::scoped_lock lock(g_mutex);
  g_buffered_diagnostics.clear();
}

auto ClearDiagnosticsDedup() -> void {
  std::scoped_lock lock(g_mutex);
  g_dedup_set.clear();
}

auto GetCurrentRunErrorLogPath() -> std::string {
  return GetErrorReportDestinationLabel();
}

auto GetBufferedDiagnosticsSummary(DiagnosticSeverity minimum_severity,
                                   std::size_t max_entries) -> std::string {
  if (max_entries == 0) {
    return "";
  }

  std::vector<std::string> lines;
  lines.reserve(max_entries);
  {
    std::scoped_lock lock(g_mutex);
    const int kMinRank = ToSeverityRank(minimum_severity);
    for (const auto& buffered_diagnostic :
         std::views::reverse(g_buffered_diagnostics)) {
      if (ToSeverityRank(buffered_diagnostic.severity) < kMinRank) {
        continue;
      }

      const std::string kLine =
          "[" + std::string(ToSeverityLabel(buffered_diagnostic.severity)) +
          "] " + buffered_diagnostic.message;
      lines.push_back(kLine);
      if (lines.size() >= max_entries) {
        break;
      }
    }
  }

  if (lines.empty()) {
    return "";
  }

  std::ranges::reverse(lines);
  std::ostringstream stream;
  for (std::size_t idx = 0; idx < lines.size(); ++idx) {
    if (idx > 0) {
      stream << '\n';
    }
    stream << lines[idx];
  }
  return stream.str();
}

}  // namespace tracer_core::domain::ports
