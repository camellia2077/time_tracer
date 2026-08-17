// infrastructure/tests/android_runtime/android_runtime_insights_consistency_structure_tests.cpp
#include <iostream>

#include "application/aggregate_runtime/i_tracer_core_runtime.hpp"
#include "application/dto/insights_requests.hpp"
#include "infrastructure/tests/android_runtime/android_runtime_insights_consistency_internal.hpp"
#include "infrastructure/tests/android_runtime/android_runtime_smoke_internal.hpp"

using tracer::core::application::use_cases::ITracerCoreRuntime;

namespace android_runtime_tests::insights_consistency_internal {
namespace {

using tracer_core::core::dto::InsightsDisplayMode;
using tracer_core::core::dto::TemporalSelectionKind;

auto TestStructureLayerMdSectionIntegrity(
    const std::shared_ptr<ITracerCoreRuntime>& runtime_api, int& failures)
    -> void {
  const auto day_result = runtime_api->insights().RunTemporalInsightsQuery(
      {.display_mode = InsightsDisplayMode::kDay,
       .selection = {.kind = TemporalSelectionKind::kSingleDay,
                     .date = "2025-01-03"},
       .format = InsightsFormat::kMarkdown});
  if (!day_result.ok) {
    ++failures;
    std::cerr << "[FAIL] StructureLayer/Day: RunTemporalInsightsQuery should "
                 "succeed: "
              << day_result.error_message << '\n';
    return;
  }

  const std::string& day_md = day_result.content;

  struct SectionCheck {
    const char* label;
    const char* marker;
  };

  const SectionCheck day_checks[] = {
      {"Summary title", "# Summary"},
      {"Timeline section", "## Timeline"},
      {"Breakdown section", "## Breakdown"},
      {"Period label", "- **Period**:"},
      {"Total Time Recorded label", "- **Total Time Recorded**:"},
  };

  for (const auto& check : day_checks) {
    if (!Contains(day_md, check.marker)) {
      ++failures;
      std::cerr << "[FAIL] StructureLayer/Day: MD should contain "
                << check.label << " ('" << check.marker << "').\n";
    }
  }

  const auto month_result = runtime_api->insights().RunTemporalInsightsQuery(
      {.display_mode = InsightsDisplayMode::kMonth,
       .selection = {.kind = TemporalSelectionKind::kDateRange,
                     .start_date = "2025-01-01",
                     .end_date = "2025-01-31"},
       .format = InsightsFormat::kMarkdown});
  if (!month_result.ok) {
    ++failures;
    std::cerr << "[FAIL] StructureLayer/Month: RunTemporalInsightsQuery should "
                 "succeed: "
              << month_result.error_message << '\n';
    return;
  }

  const std::string& month_md = month_result.content;

  const SectionCheck month_checks[] = {
      {"Summary title", "# Summary"},
      {"Breakdown section", "## Breakdown"},
      {"Total Time Recorded label", "- **Total Time Recorded**:"},
  };

  for (const auto& check : month_checks) {
    if (!Contains(month_md, check.marker)) {
      ++failures;
      std::cerr << "[FAIL] StructureLayer/Month: MD should contain "
                << check.label << " ('" << check.marker << "').\n";
    }
  }
}

}  // namespace

auto RunInsightsConsistencyStructureTests(
    const std::shared_ptr<ITracerCoreRuntime>& runtime_api, int& failures)
    -> void {
  TestStructureLayerMdSectionIntegrity(runtime_api, failures);
}

}  // namespace android_runtime_tests::insights_consistency_internal
