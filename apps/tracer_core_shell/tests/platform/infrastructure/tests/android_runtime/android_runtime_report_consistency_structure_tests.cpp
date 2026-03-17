// infrastructure/tests/android_runtime/android_runtime_report_consistency_structure_tests.cpp
#include <iostream>

#include "application/dto/core_requests.hpp"
#include "application/use_cases/i_tracer_core_api.hpp"
#include "infrastructure/tests/android_runtime/android_runtime_report_consistency_internal.hpp"
#include "infrastructure/tests/android_runtime/android_runtime_smoke_internal.hpp"

using tracer::core::application::use_cases::ITracerCoreApi;

namespace android_runtime_tests::report_consistency_internal {
namespace {

auto TestStructureLayerMdSectionIntegrity(
    const std::shared_ptr<ITracerCoreApi>& core_api, int& failures) -> void {
  tracer_core::core::dto::ReportQueryRequest day_request;
  day_request.type = tracer_core::core::dto::ReportQueryType::kDay;
  day_request.argument = "2025-01-03";
  day_request.format = ReportFormat::kMarkdown;

  const auto day_result = core_api->RunReportQuery(day_request);
  if (!day_result.ok) {
    ++failures;
    std::cerr << "[FAIL] StructureLayer/Day: RunReportQuery should succeed: "
              << day_result.error_message << '\n';
    return;
  }

  const std::string& day_md = day_result.content;

  struct SectionCheck {
    const char* label;
    const char* marker;
  };

  const SectionCheck day_checks[] = {
      {"title (Daily Report for)", "# Daily Report for"},
      {"All Activities section", "## All Activities"},
      {"Project Breakdown section", "## Project Breakdown"},
      {"Date label", "- **Date**:"},
      {"Total Time Recorded label", "- **Total Time Recorded**:"},
      {"Statistics section", "## Statistics"},
  };

  for (const auto& check : day_checks) {
    if (!Contains(day_md, check.marker)) {
      ++failures;
      std::cerr << "[FAIL] StructureLayer/Day: MD should contain "
                << check.label << " ('" << check.marker << "').\n";
    }
  }

  tracer_core::core::dto::ReportQueryRequest month_request;
  month_request.type = tracer_core::core::dto::ReportQueryType::kMonth;
  month_request.argument = "2025-01";
  month_request.format = ReportFormat::kMarkdown;

  const auto month_result = core_api->RunReportQuery(month_request);
  if (!month_result.ok) {
    ++failures;
    std::cerr << "[FAIL] StructureLayer/Month: RunReportQuery should succeed: "
              << month_result.error_message << '\n';
    return;
  }

  const std::string& month_md = month_result.content;

  const SectionCheck month_checks[] = {
      {"title (Monthly Summary)", "# Monthly Summary for"},
      {"Project Breakdown section", "## Project Breakdown"},
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

auto RunReportConsistencyStructureTests(
    const std::shared_ptr<ITracerCoreApi>& core_api, int& failures) -> void {
  TestStructureLayerMdSectionIntegrity(core_api, failures);
}

}  // namespace android_runtime_tests::report_consistency_internal
