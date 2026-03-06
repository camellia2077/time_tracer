import tracer.core.application;

#include <filesystem>
#include <iostream>
#include <string_view>
#include <type_traits>

namespace {

using tracer::core::application::modimporter::ImportService;
using tracer::core::application::modimporter::ImportStats;
using tracer::core::application::modimporter::ReplaceMonthTarget;
using tracer::core::application::modpipeline::ConverterStep;
using tracer::core::application::modpipeline::FileCollector;
using tracer::core::application::modpipeline::LogicLinkerStep;
using tracer::core::application::modpipeline::LogicValidatorStep;
using tracer::core::application::modpipeline::PipelineContext;
using tracer::core::application::modpipeline::PipelineManager;
using tracer::core::application::modpipeline::PipelineRunConfig;
using tracer::core::application::modpipeline::StructureValidatorStep;
using tracer::core::application::modservice::ConverterService;
using tracer::core::application::modusecases::ITracerCoreApi;
using tracer::core::application::modusecases::TracerCoreApi;
using tracer::core::application::modworkflow::IWorkflowHandler;
using tracer::core::application::modworkflow::WorkflowHandler;

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
  Expect(std::is_class_v<PipelineRunConfig>,
         "PipelineRunConfig should be visible through module bridge.",
         failures);
  Expect(std::is_class_v<PipelineContext>,
         "PipelineContext should be visible through module bridge.", failures);
  Expect(std::is_class_v<PipelineManager>,
         "PipelineManager should be visible through module bridge.", failures);
  Expect(std::is_class_v<FileCollector>,
         "FileCollector should be visible through module bridge.", failures);
  Expect(std::is_class_v<StructureValidatorStep>,
         "StructureValidatorStep should be visible through module bridge.",
         failures);
  Expect(std::is_class_v<ConverterStep>,
         "ConverterStep should be visible through module bridge.", failures);
  Expect(std::is_class_v<LogicLinkerStep>,
         "LogicLinkerStep should be visible through module bridge.", failures);
  Expect(std::is_class_v<LogicValidatorStep>,
         "LogicValidatorStep should be visible through module bridge.",
         failures);

  PipelineContext context(std::filesystem::path("phase4-module-output"));
  Expect(context.config.output_root ==
             std::filesystem::path("phase4-module-output"),
         "PipelineContext constructor should preserve output_root.", failures);
  Expect(context.state.ingest_inputs.empty(),
         "PipelineContext state should initialize as empty.", failures);
  Expect(context.result.processed_data.empty(),
         "PipelineContext result should initialize as empty.", failures);
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
