import tracer.core.application;

#include <filesystem>
#include <iostream>
#include <string_view>
#include <type_traits>

namespace {

using tracer::core::application::modimporter::ImportService;
using tracer::core::application::modimporter::ImportStats;
using tracer::core::application::modimporter::ReplaceMonthTarget;
using tracer::core::application::modservice::ConverterService;
using tracer::core::application::modusecases::ITracerCoreApi;
using tracer::core::application::modusecases::TracerCoreApi;
using tracer::core::application::modworkflow::IWorkflowHandler;
using tracer::core::application::modworkflow::WorkflowHandler;

namespace app_pipeline = tracer::core::application::pipeline;

auto Expect(bool condition, std::string_view message, int& failures) -> void {
  if (condition) {
    return;
  }
  ++failures;
  std::cerr << "[FAIL] " << message << '\n';
}

void TestUseCasesAndServices(int& failures) {
  Expect(std::is_class_v<ITracerCoreApi>,
         "ITracerCoreApi should be visible through module bridge.", failures);
  Expect(std::is_abstract_v<ITracerCoreApi>,
         "ITracerCoreApi should remain an abstract interface.", failures);
  Expect(std::is_base_of_v<ITracerCoreApi, TracerCoreApi>,
         "TracerCoreApi should keep ITracerCoreApi inheritance.", failures);
  Expect(std::is_class_v<ConverterService>,
         "ConverterService should be visible through module bridge.", failures);
  Expect(std::is_class_v<ImportService>,
         "ImportService should be visible through module bridge.", failures);

  ImportStats stats;
  ReplaceMonthTarget target{.kYear = 2026, .kMonth = 3};
  Expect(stats.total_files == 0U && target.kMonth == 3,
         "Importer DTOs should be visible through module bridge.", failures);
}

void TestPipelineBridge(int& failures) {
  Expect(std::is_class_v<app_pipeline::PipelineRunSpec>,
         "PipelineRunSpec should be visible through the pipeline module.",
         failures);
  Expect(std::is_class_v<app_pipeline::PipelineSession>,
         "PipelineSession should be visible through the pipeline module.",
         failures);
  Expect(std::is_class_v<app_pipeline::PipelineOrchestrator>,
         "PipelineOrchestrator should be visible through the pipeline module.",
         failures);
  Expect(std::is_class_v<app_pipeline::InputCollectionStage>,
         "InputCollectionStage should be visible through the pipeline module.",
         failures);
  Expect(std::is_class_v<app_pipeline::StructureValidationStage>,
         "StructureValidationStage should be visible through the pipeline module.",
         failures);
  Expect(std::is_class_v<app_pipeline::ConversionStage>,
         "ConversionStage should be visible through the pipeline module.",
         failures);
  Expect(std::is_class_v<app_pipeline::CrossMonthLinkStage>,
         "CrossMonthLinkStage should be visible through the pipeline module.",
         failures);
  Expect(std::is_class_v<app_pipeline::LogicValidationStage>,
         "LogicValidationStage should be visible through the pipeline module.",
         failures);

  app_pipeline::PipelineSession context(
      std::filesystem::path("phase5-pipeline-module-output"));
  Expect(context.config.output_root ==
             std::filesystem::path("phase5-pipeline-module-output"),
         "PipelineSession constructor should preserve output_root.", failures);
  Expect(context.state.ingest_inputs.empty(),
         "PipelineSession state should initialize as empty.", failures);
  Expect(context.result.processed_data.empty(),
         "PipelineSession result should initialize as empty.", failures);
}

void TestWorkflowBridge(int& failures) {
  Expect(std::is_base_of_v<IWorkflowHandler, WorkflowHandler>,
         "WorkflowHandler should keep IWorkflowHandler inheritance.", failures);
}

}  // namespace

auto main() -> int {
  int failures = 0;
  TestUseCasesAndServices(failures);
  TestPipelineBridge(failures);
  TestWorkflowBridge(failures);

  if (failures == 0) {
    std::cout << "[PASS] tracer_core_application_modules_smoke_tests\n";
    return 0;
  }

  std::cerr << "[FAIL] tracer_core_application_modules_smoke_tests failures: "
            << failures << '\n';
  return 1;
}
