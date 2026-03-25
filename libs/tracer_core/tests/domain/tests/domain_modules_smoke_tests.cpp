import tracer.core.domain;

#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace {

using tracer::core::domain::model::BaseActivityRecord;
using tracer::core::domain::model::DailyLog;
using tracer::core::domain::model::ProcessingResult;
using tracer::core::domain::model::SourceSpan;
using tracer::core::domain::moderrors::ErrorRecord;
using tracer::core::domain::moderrors::ErrorSeverity;
using tracer::core::domain::modreports::DailyReportData;
using tracer::core::domain::modreports::FormattedYearlyReports;
using tracer::core::domain::modreports::PeriodReportData;
using tracer::core::domain::modreports::ProjectTree;
using tracer::core::domain::modreports::ReportFormat;
using tracer::core::domain::modreports::WeeklyReportData;
using tracer::core::domain::modrepos::IProjectRepository;
using tracer::core::domain::modrepos::ProjectEntity;
using tracer::core::domain::ports::AppendErrorReport;
using tracer::core::domain::ports::ClearBufferedDiagnostics;
using tracer::core::domain::ports::ClearDiagnosticsDedup;
using tracer::core::domain::ports::DiagnosticSeverity;
using tracer::core::domain::ports::EmitWarn;
using tracer::core::domain::ports::GetBufferedDiagnosticsSummary;
using tracer::core::domain::ports::GetErrorReportDestinationLabel;
using tracer::core::domain::ports::IDiagnosticsSink;
using tracer::core::domain::ports::IErrorReportWriter;
using tracer::core::domain::ports::SetDiagnosticsSink;
using tracer::core::domain::ports::SetErrorReportWriter;
using tracer::core::domain::types::AppOptions;
using tracer::core::domain::types::ConverterConfig;
using tracer::core::domain::types::DateCheckMode;
using tracer::core::domain::types::IngestMode;

auto Expect(bool condition, std::string_view message, int& failures) -> void {
  if (condition) {
    return;
  }
  ++failures;
  std::cerr << "[FAIL] " << message << '\n';
}

class FakeDiagnosticsSink final : public IDiagnosticsSink {
 public:
  auto Emit(DiagnosticSeverity severity, std::string_view message)
      -> void override {
    records.emplace_back(static_cast<int>(severity), std::string(message));
  }

  std::vector<std::pair<int, std::string>> records;
};

class FakeErrorWriter final : public IErrorReportWriter {
 public:
  auto Append(std::string_view report_content) -> bool override {
    appended.emplace_back(report_content);
    return true;
  }

  [[nodiscard]] auto DestinationLabel() const -> std::string override {
    return "fake-writer";
  }

  std::vector<std::string> appended;
};

class FakeProjectRepository final : public IProjectRepository {
 public:
  auto GetAllProjects() -> std::vector<ProjectEntity> override {
    return {{.id = 1, .parent_id = std::nullopt, .name = "root"}};
  }
};

void TestPortsAndRepository(int& failures) {
  ClearBufferedDiagnostics();
  ClearDiagnosticsDedup();

  auto sink = std::make_shared<FakeDiagnosticsSink>();
  auto writer = std::make_shared<FakeErrorWriter>();
  SetDiagnosticsSink(sink);
  SetErrorReportWriter(writer);

  EmitWarn("phase2-domain-smoke");
  Expect(!sink->records.empty(), "Diagnostic sink should receive records.",
         failures);

  Expect(AppendErrorReport("phase2 error report"),
         "Error writer append should return true.", failures);
  Expect(GetErrorReportDestinationLabel() == "fake-writer",
         "Error writer label mismatch.", failures);

  const auto summary =
      GetBufferedDiagnosticsSummary(DiagnosticSeverity::kInfo, 8U);
  Expect(!summary.empty(), "Diagnostics summary should not be empty.",
         failures);

  FakeProjectRepository repo;
  const auto projects = repo.GetAllProjects();
  Expect(projects.size() == 1U && projects[0].name == "root",
         "Project repository interface mismatch.", failures);
}

void TestDomainModelsAndTypes(int& failures) {
  AppOptions options;
  options.convert = true;
  options.date_check_mode = DateCheckMode::kContinuity;
  Expect(options.convert, "AppOptions convert flag mismatch.", failures);
  Expect(options.date_check_mode == DateCheckMode::kContinuity,
         "AppOptions date_check_mode mismatch.", failures);

  ConverterConfig config;
  config.remark_prefix = "#";
  config.header_order = {"sleep", "study"};
  Expect(config.header_order.size() == 2U,
         "ConverterConfig header_order mismatch.", failures);
  const IngestMode mode = IngestMode::kStandard;
  Expect(mode == IngestMode::kStandard, "IngestMode mismatch.", failures);

  SourceSpan span;
  span.file_path = "sample.txt";
  span.line_start = 10;
  span.raw_text = "line";
  Expect(span.HasLine(), "SourceSpan HasLine should be true.", failures);

  BaseActivityRecord record;
  record.project_path = "study_cpp";
  record.duration_seconds = 1800;
  record.source_span = span;
  DailyLog daily;
  daily.date = "2026-03-04";
  daily.processedActivities.push_back(record);
  Expect(daily.processedActivities.size() == 1U,
         "DailyLog processedActivities mismatch.", failures);

  ProcessingResult result;
  result.timings.validation_source_ms = 1.0;
  Expect(result.success, "ProcessingResult success default mismatch.",
         failures);

  ErrorRecord error;
  error.code = "validation.line.invalid";
  error.severity = ErrorSeverity::kWarn;
  error.message = "warn";
  Expect(error.severity == ErrorSeverity::kWarn,
         "ErrorRecord severity mismatch.", failures);
}

void TestReportModels(int& failures) {
  ProjectTree tree;
  tree["study"].duration = 120;
  tree["study"].children["cpp"].duration = 60;
  Expect(tree["study"].children.size() == 1U, "ProjectTree children mismatch.",
         failures);

  PeriodReportData period;
  period.range_label = "2026-W10";
  period.total_duration = 7200;
  Expect(period.total_duration == 7200, "PeriodReportData mismatch.", failures);

  WeeklyReportData weekly;
  weekly.actual_days = 7;
  Expect(weekly.actual_days == 7, "WeeklyReportData mismatch.", failures);

  DailyReportData daily;
  daily.date = "2026-03-04";
  daily.project_tree = tree;
  Expect(daily.project_tree.size() == 1U,
         "DailyReportData project_tree mismatch.", failures);

  FormattedYearlyReports yearly_reports;
  yearly_reports.emplace(2026, "ok");
  Expect(yearly_reports.size() == 1U, "FormattedYearlyReports mismatch.",
         failures);

  const auto format = ReportFormat::kMarkdown;
  Expect(format == ReportFormat::kMarkdown, "ReportFormat mismatch.", failures);
}

}  // namespace

auto main() -> int {
  int failures = 0;
  TestPortsAndRepository(failures);
  TestDomainModelsAndTypes(failures);
  TestReportModels(failures);

  if (failures == 0) {
    std::cout << "[PASS] tracer_core_domain_modules_smoke_tests\n";
    return 0;
  }

  std::cerr << "[FAIL] tracer_core_domain_modules_smoke_tests failures: "
            << failures << '\n';
  return 1;
}
