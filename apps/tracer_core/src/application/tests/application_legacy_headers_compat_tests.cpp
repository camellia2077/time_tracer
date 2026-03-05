#include "application/importer/import_service.hpp"
#include "application/pipeline/context/pipeline_context.hpp"
#include "application/pipeline/pipeline_manager.hpp"
#include "application/pipeline/steps/pipeline_stages.hpp"
#include "application/service/converter_service.hpp"
#include "application/use_cases/i_tracer_core_api.hpp"
#include "application/use_cases/tracer_core_api.hpp"
#include "application/workflow_handler.hpp"

#include <filesystem>
#include <iostream>
#include <string_view>
#include <type_traits>

namespace {

auto Expect(bool condition, std::string_view message, int& failures) -> void {
  if (condition) {
    return;
  }
  ++failures;
  std::cerr << "[FAIL] " << message << '\n';
}

void TestLegacyUseCasesAndServices(int& failures) {
  Expect(std::is_class_v<ITracerCoreApi>,
         "Legacy ITracerCoreApi header path should remain visible.", failures);
  Expect(std::is_abstract_v<ITracerCoreApi>,
         "Legacy ITracerCoreApi should remain abstract.", failures);
  Expect(std::is_base_of_v<ITracerCoreApi, TracerCoreApi>,
         "Legacy TracerCoreApi inheritance should remain stable.", failures);
  Expect(std::is_class_v<ConverterService>,
         "Legacy ConverterService header path should remain visible.",
         failures);
  Expect(std::is_class_v<ImportService>,
         "Legacy ImportService header path should remain visible.", failures);

  ImportStats stats;
  ReplaceMonthTarget target{.kYear = 2026, .kMonth = 3};
  Expect(stats.total_files == 0U && target.kYear == 2026,
         "Legacy importer DTOs should remain visible.", failures);
}

void TestLegacyPipelineHeaders(int& failures) {
  Expect(std::is_class_v<core::pipeline::PipelineRunConfig>,
         "Legacy PipelineRunConfig header path should remain visible.",
         failures);
  Expect(std::is_class_v<core::pipeline::PipelineContext>,
         "Legacy PipelineContext header path should remain visible.", failures);
  Expect(std::is_class_v<core::pipeline::PipelineManager>,
         "Legacy PipelineManager header path should remain visible.", failures);
  Expect(std::is_class_v<core::pipeline::FileCollector>,
         "Legacy FileCollector header path should remain visible.", failures);
  Expect(std::is_class_v<core::pipeline::StructureValidatorStep>,
         "Legacy StructureValidatorStep header path should remain visible.",
         failures);
  Expect(std::is_class_v<core::pipeline::ConverterStep>,
         "Legacy ConverterStep header path should remain visible.", failures);
  Expect(std::is_class_v<core::pipeline::LogicLinkerStep>,
         "Legacy LogicLinkerStep header path should remain visible.", failures);
  Expect(std::is_class_v<core::pipeline::LogicValidatorStep>,
         "Legacy LogicValidatorStep header path should remain visible.",
         failures);

  core::pipeline::PipelineContext context(
      std::filesystem::path("phase4-legacy-output"));
  Expect(context.config.output_root ==
             std::filesystem::path("phase4-legacy-output"),
         "Legacy PipelineContext constructor should preserve output_root.",
         failures);
  Expect(context.state.generated_files.empty(),
         "Legacy PipelineContext state should initialize as empty.", failures);
}

void TestLegacyWorkflowHeaders(int& failures) {
  Expect(std::is_base_of_v<IWorkflowHandler, WorkflowHandler>,
         "Legacy WorkflowHandler inheritance should remain stable.", failures);
}

}  // namespace

auto main() -> int {
  int failures = 0;
  TestLegacyUseCasesAndServices(failures);
  TestLegacyPipelineHeaders(failures);
  TestLegacyWorkflowHeaders(failures);

  if (failures == 0) {
    std::cout << "[PASS] tracer_core_application_legacy_headers_compat_tests\n";
    return 0;
  }

  std::cerr
      << "[FAIL] tracer_core_application_legacy_headers_compat_tests failures: "
      << failures << '\n';
  return 1;
}
