// infrastructure/tests/android_runtime/android_runtime_report_consistency_tests.cpp
#include <exception>
#include <iostream>

#include "application/dto/reporting_requests.hpp"
#include "infrastructure/tests/android_runtime/android_runtime_report_consistency_internal.hpp"
#include "infrastructure/tests/android_runtime/android_runtime_smoke_internal.hpp"

namespace android_runtime_tests {

auto RunReportConsistencyTests(int& failures) -> void {
  auto fixture_opt = smoke::BuildRuntimeFixture(
      "time_tracer_report_consistency_test", failures);
  if (!fixture_opt.has_value()) {
    return;
  }

  auto fixture = std::move(*fixture_opt);
  try {
    tracer_core::core::dto::IngestRequest ingest_request;
    ingest_request.input_path = (fixture.input_path / "2025").string();
    ingest_request.date_check_mode = DateCheckMode::kNone;
    const auto ingest_result =
        fixture.runtime.runtime_api->pipeline().RunIngest(ingest_request);
    if (!ingest_result.ok) {
      ++failures;
      std::cerr << "[FAIL] ReportConsistency: initial ingest should succeed: "
                << ingest_result.error_message << '\n';
      smoke::CleanupRuntimeFixture(fixture);
      return;
    }

    report_consistency_internal::RunReportConsistencyFieldVerificationTests(
        fixture.runtime.runtime_api, failures);
    report_consistency_internal::RunReportConsistencyStructureTests(
        fixture.runtime.runtime_api, failures);
    report_consistency_internal::RunReportConsistencyCrossIngestTests(
        fixture.runtime.runtime_api, fixture.input_path, failures);
  } catch (const std::exception& exception) {
    ++failures;
    std::cerr << "[FAIL] ReportConsistency tests threw exception: "
              << exception.what() << '\n';
  } catch (...) {
    ++failures;
    std::cerr << "[FAIL] ReportConsistency tests threw non-standard "
                 "exception.\n";
  }

  smoke::CleanupRuntimeFixture(fixture);
}

}  // namespace android_runtime_tests
