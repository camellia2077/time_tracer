// infrastructure/tests/android_runtime/android_runtime_smoke_io_tests.cpp
#include <exception>
#include <iostream>

#include "infrastructure/tests/android_runtime/android_runtime_smoke_io_internal.hpp"

namespace android_runtime_tests::smoke {

auto RunIoSmokeSection(int& failures) -> void {
  auto fixture_opt = BuildRuntimeFixture(
      "time_tracer_android_runtime_smoke_io_test", failures);
  if (!fixture_opt.has_value()) {
    return;
  }

  RuntimeFixture fixture = std::move(*fixture_opt);
  try {
    tracer_core::core::dto::IngestRequest ingest_request;
    ingest_request.input_path = fixture.input_path.string();
    ingest_request.date_check_mode = DateCheckMode::kNone;
    const auto ingest_result =
        fixture.runtime.core_api->RunIngest(ingest_request);
    if (!ingest_result.ok) {
      ++failures;
      std::cerr << "[FAIL] RunIngest should succeed for report-chart test: "
                << ingest_result.error_message << '\n';
    } else {
      ChartProbeContext chart_probe;
      ProbeChartRange(fixture.runtime.core_api, chart_probe, failures);
      VerifyExplicitChartRange(fixture.runtime.core_api, chart_probe, failures);
      VerifyChartForRootScenarios(fixture.runtime.core_api, chart_probe,
                                  failures);
    }

    VerifyReportOutputs(fixture.runtime.core_api, failures);
  } catch (const std::exception& exception) {
    ++failures;
    std::cerr << "[FAIL] Android runtime IO smoke test threw exception: "
              << exception.what() << '\n';
  } catch (...) {
    ++failures;
    std::cerr << "[FAIL] Android runtime IO smoke test threw non-standard "
                 "exception.\n";
  }

  CleanupRuntimeFixture(fixture);
}

}  // namespace android_runtime_tests::smoke
