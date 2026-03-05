#include "domain/errors/error_record.hpp"
#include "domain/model/daily_log.hpp"
#include "domain/model/processing_result.hpp"
#include "domain/model/source_span.hpp"
#include "domain/model/time_data_models.hpp"
#include "domain/ports/diagnostics.hpp"
#include "domain/reports/models/daily_report_data.hpp"
#include "domain/reports/models/period_report_models.hpp"
#include "domain/reports/models/project_tree.hpp"
#include "domain/reports/models/query_data_structs.hpp"
#include "domain/reports/types/report_types.hpp"
#include "domain/repositories/i_project_repository.hpp"
#include "domain/types/app_options.hpp"
#include "domain/types/converter_config.hpp"
#include "domain/types/date_check_mode.hpp"
#include "domain/types/ingest_mode.hpp"

#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace {

auto Expect(bool condition, std::string_view message, int& failures) -> void {
  if (condition) {
    return;
  }
  ++failures;
  std::cerr << "[FAIL] " << message << '\n';
}

class FakeDiagnosticsSink final : public tracer_core::domain::ports::IDiagnosticsSink {
 public:
  auto Emit(tracer_core::domain::ports::DiagnosticSeverity severity,
            std::string_view message) -> void override {
    records.emplace_back(static_cast<int>(severity), std::string(message));
  }

  std::vector<std::pair<int, std::string>> records;
};

class FakeProjectRepository final : public IProjectRepository {
 public:
  auto GetAllProjects() -> std::vector<ProjectEntity> override {
    return {{.id = 2, .parent_id = 1, .name = "child"}};
  }
};

void TestLegacyHeaderPath(int& failures) {
  AppOptions options;
  options.validate_logic = true;
  options.date_check_mode = DateCheckMode::kFull;
  Expect(options.validate_logic, "Legacy AppOptions mismatch.", failures);

  ConverterConfig config;
  config.text_mapping["a"] = "b";
  Expect(config.text_mapping.size() == 1U, "Legacy ConverterConfig mismatch.",
         failures);
  const IngestMode mode = IngestMode::kSingleTxtReplaceMonth;
  Expect(mode == IngestMode::kSingleTxtReplaceMonth,
         "Legacy IngestMode mismatch.", failures);

  SourceSpan span;
  span.file_path = "legacy.txt";
  span.line_start = 1;
  Expect(span.HasLine(), "Legacy SourceSpan mismatch.", failures);

  BaseActivityRecord record;
  record.source_span = span;
  ActivityStats stats;
  stats.study_time = 30;
  DailyLog log;
  log.processedActivities.push_back(record);
  log.stats = stats;
  Expect(log.stats.study_time == 30, "Legacy DailyLog mismatch.", failures);

  ProcessingResult result;
  result.success = true;
  Expect(result.success, "Legacy ProcessingResult mismatch.", failures);

  tracer_core::domain::errors::ErrorRecord error;
  error.code = "legacy.code";
  error.severity = tracer_core::domain::errors::ErrorSeverity::kError;
  Expect(error.code == "legacy.code", "Legacy ErrorRecord mismatch.", failures);

  auto sink = std::make_shared<FakeDiagnosticsSink>();
  tracer_core::domain::ports::SetDiagnosticsSink(sink);
  tracer_core::domain::ports::EmitWarn("legacy warning");
  Expect(!sink->records.empty(), "Legacy diagnostics sink mismatch.", failures);

  FakeProjectRepository repo;
  const auto projects = repo.GetAllProjects();
  Expect(!projects.empty() && projects[0].name == "child",
         "Legacy repository interface mismatch.", failures);

  reporting::ProjectTree tree;
  tree["legacy"].duration = 10;
  PeriodReportData period;
  period.project_tree = tree;
  DailyReportData daily;
  daily.project_tree = tree;
  FormattedYearlyReports yearly;
  yearly.emplace(2026, "legacy");
  Expect(yearly.size() == 1U && daily.project_tree.size() == 1U,
         "Legacy report models mismatch.", failures);

  const auto format = ReportFormat::kMarkdown;
  Expect(format == ReportFormat::kMarkdown, "Legacy report format mismatch.",
         failures);
}

}  // namespace

auto main() -> int {
  int failures = 0;
  TestLegacyHeaderPath(failures);

  if (failures == 0) {
    std::cout << "[PASS] tracer_core_domain_legacy_headers_compat_tests\n";
    return 0;
  }

  std::cerr << "[FAIL] tracer_core_domain_legacy_headers_compat_tests failures: "
            << failures << '\n';
  return 1;
}
