import tracer.core.application;

#include <filesystem>
#include <iostream>
#include <string_view>
#include <type_traits>

namespace {

namespace app_pipeline = tracer::core::application::pipeline;

auto Expect(bool condition, std::string_view message, int& failures) -> void {
  if (condition) {
    return;
  }
  ++failures;
  std::cerr << "[FAIL] " << message << '\n';
}

auto RunPipelineModuleSmoke(int& failures) -> void {
  Expect(std::is_class_v<app_pipeline::PipelineRunSpec>,
         "PipelineRunSpec should be visible through pipeline module.",
         failures);
  Expect(std::is_class_v<app_pipeline::PipelineSession>,
         "PipelineSession should be visible through pipeline module.",
         failures);
  Expect(std::is_class_v<app_pipeline::PipelineOrchestrator>,
         "PipelineOrchestrator should be visible through pipeline module.",
         failures);
  Expect(std::is_class_v<app_pipeline::InputCollectionStage>,
         "InputCollectionStage should be visible through pipeline module.",
         failures);
  Expect(std::is_class_v<app_pipeline::StructureValidationStage>,
         "StructureValidationStage should be visible through pipeline module.",
         failures);
  Expect(std::is_class_v<app_pipeline::ConversionStage>,
         "ConversionStage should be visible through pipeline module.",
         failures);
  Expect(std::is_class_v<app_pipeline::CrossMonthLinkStage>,
         "CrossMonthLinkStage should be visible through pipeline module.",
         failures);
  Expect(std::is_class_v<app_pipeline::LogicValidationStage>,
         "LogicValidationStage should be visible through pipeline module.",
         failures);

  app_pipeline::PipelineSession context(
      std::filesystem::path("phase5-pipeline-module-output"));
  Expect(context.config.output_root ==
             std::filesystem::path("phase5-pipeline-module-output"),
         "PipelineSession should preserve output_root.", failures);
  Expect(context.state.ingest_inputs.empty(),
         "PipelineSession state should initialize empty.", failures);
  Expect(context.result.processed_data.empty(),
         "PipelineSession result should initialize empty.", failures);
}

}  // namespace

auto main() -> int {
  int failures = 0;
  RunPipelineModuleSmoke(failures);
  if (failures == 0) {
    std::cout << "[PASS] tracer_core_application_pipeline_module_smoke_tests\n";
    return 0;
  }
  std::cerr
      << "[FAIL] tracer_core_application_pipeline_module_smoke_tests failures: "
      << failures << '\n';
  return 1;
}
