// application/pipeline/steps/pipeline_stages.hpp
#ifndef APPLICATION_PIPELINE_STEPS_PIPELINE_STAGES_H_
#define APPLICATION_PIPELINE_STEPS_PIPELINE_STAGES_H_

#include <string>

#include "application/pipeline/context/pipeline_context.hpp"
#include "application/ports/i_ingest_input_provider.hpp"

namespace core::pipeline {

class FileCollector {
 public:
  static auto Execute(
      PipelineContext& context,
      const tracer_core::application::ports::IIngestInputProvider&
          input_provider,
      const std::string& extension = ".txt") -> bool;
};

class StructureValidatorStep {
 public:
  static auto Execute(PipelineContext& context) -> bool;
};

class ConverterStep {
 public:
  static auto Execute(PipelineContext& context) -> bool;

 private:
  static void PrintTiming(double total_time_ms);
};

class LogicLinkerStep {
 public:
  static auto Execute(PipelineContext& context) -> bool;
};

class LogicValidatorStep {
 public:
  static auto Execute(PipelineContext& context) -> bool;
};

}  // namespace core::pipeline

#endif  // APPLICATION_PIPELINE_STEPS_PIPELINE_STAGES_H_
