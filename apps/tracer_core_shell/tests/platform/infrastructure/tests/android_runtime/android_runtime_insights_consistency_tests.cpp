// infrastructure/tests/android_runtime/android_runtime_insights_consistency_tests.cpp
#include <exception>
#include <iostream>

#include "application/dto/insights_requests.hpp"
#include "infrastructure/tests/android_runtime/android_runtime_insights_consistency_internal.hpp"
#include "infrastructure/tests/android_runtime/android_runtime_smoke_internal.hpp"

namespace android_runtime_tests {

auto RunInsightsConsistencyTests(int& failures) -> void {
  auto fixture_opt = smoke::BuildRuntimeFixture(
      "time_tracer_insights_consistency_test", failures);
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
      std::cerr << "[FAIL] InsightsConsistency: initial ingest should succeed: "
                << ingest_result.error_message << '\n';
      smoke::CleanupRuntimeFixture(fixture);
      return;
    }

    infrastructure::bootstrap::SetAndroidRuntimeStatusConfigs(
        fixture.runtime,
        {.day = {.statuses =
                     {{.id = "status", .label = "Study", .parent = "study"},
                      {.id = "exercise",
                       .label = "Exercise",
                       .parent = "exercise"}}},
         .week = {.statuses =
                      {{.id = "status", .label = "Study", .parent = "study"},
                       {.id = "exercise",
                        .label = "Exercise",
                        .parent = "exercise"}}},
         .month = {.statuses =
                       {{.id = "status", .label = "Study", .parent = "study"},
                        {.id = "exercise",
                         .label = "Exercise",
                         .parent = "exercise"}}},
         .year = {.statuses =
                      {{.id = "status", .label = "Study", .parent = "study"},
                       {.id = "exercise",
                        .label = "Exercise",
                        .parent = "exercise"}}},
         .recent = {.statuses =
                        {{.id = "status", .label = "Study", .parent = "study"},
                         {.id = "exercise",
                          .label = "Exercise",
                          .parent = "exercise"}}},
         .range = {
             .statuses = {{.id = "status", .label = "Study", .parent = "study"},
                          {.id = "exercise",
                           .label = "Exercise",
                           .parent = "exercise"}}}});

    insights_consistency_internal::RunInsightsConsistencyFieldVerificationTests(
        fixture.runtime.runtime_api, failures);
    insights_consistency_internal::RunInsightsConsistencyStructureTests(
        fixture.runtime.runtime_api, failures);
    insights_consistency_internal::RunInsightsConsistencyCrossIngestTests(
        fixture.runtime.runtime_api, fixture.input_path, failures);
  } catch (const std::exception& exception) {
    ++failures;
    std::cerr << "[FAIL] InsightsConsistency tests threw exception: "
              << exception.what() << '\n';
  } catch (...) {
    ++failures;
    std::cerr << "[FAIL] InsightsConsistency tests threw non-standard "
                 "exception.\n";
  }

  smoke::CleanupRuntimeFixture(fixture);
}

}  // namespace android_runtime_tests
