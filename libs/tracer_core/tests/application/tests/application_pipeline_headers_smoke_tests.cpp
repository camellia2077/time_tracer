#include "application/pipeline/pipeline_orchestrator.hpp"
#include "application/pipeline/pipeline_stages.hpp"
#include "application/pipeline/pipeline_types.hpp"

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

void TestPipelineHeaders(int& failures) {
  Expect(std::is_class_v<app_pipeline::PipelineRunSpec>,
         "PipelineRunSpec should be visible through the new header contract.",
         failures);
  Expect(std::is_class_v<app_pipeline::PipelineRuntimeState>,
         "PipelineRuntimeState should be visible through the new header contract.",
         failures);
  Expect(std::is_class_v<app_pipeline::PipelineOutput>,
         "PipelineOutput should be visible through the new header contract.",
         failures);
  Expect(std::is_class_v<app_pipeline::PipelineSession>,
         "PipelineSession should be visible through the new header contract.",
         failures);
  Expect(std::is_class_v<app_pipeline::PipelineOrchestrator>,
         "PipelineOrchestrator should be visible through the new header contract.",
         failures);
  Expect(std::is_class_v<app_pipeline::InputCollectionStage>,
         "InputCollectionStage should be visible through the new header contract.",
         failures);
  Expect(std::is_class_v<app_pipeline::StructureValidationStage>,
         "StructureValidationStage should be visible through the new header contract.",
         failures);
  Expect(std::is_class_v<app_pipeline::ConversionStage>,
         "ConversionStage should be visible through the new header contract.",
         failures);
  Expect(std::is_class_v<app_pipeline::CrossMonthLinkStage>,
         "CrossMonthLinkStage should be visible through the new header contract.",
         failures);
  Expect(std::is_class_v<app_pipeline::LogicValidationStage>,
         "LogicValidationStage should be visible through the new header contract.",
         failures);

  app_pipeline::PipelineSession session(
      std::filesystem::path("phase5-pipeline-header-output"));
  Expect(session.config.output_root ==
             std::filesystem::path("phase5-pipeline-header-output"),
         "PipelineSession should preserve output_root through the new header contract.",
         failures);
}

}  // namespace

auto main() -> int {
  int failures = 0;
  TestPipelineHeaders(failures);

  if (failures == 0) {
    std::cout << "[PASS] tracer_core_application_pipeline_headers_smoke_tests\n";
    return 0;
  }

  std::cerr
      << "[FAIL] tracer_core_application_pipeline_headers_smoke_tests failures: "
      << failures << '\n';
  return 1;
}
