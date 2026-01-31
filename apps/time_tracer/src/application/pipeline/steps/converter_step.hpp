// application/pipeline/steps/converter_step.hpp
#ifndef APPLICATION_PIPELINE_STEPS_CONVERTER_STEP_H_
#define APPLICATION_PIPELINE_STEPS_CONVERTER_STEP_H_

#include "common/config/app_config.hpp"
#include "application/pipeline/context/pipeline_context.hpp"

namespace core::pipeline {

class ConverterStep {
 public:
  explicit ConverterStep(const AppConfig& config);
  bool execute(PipelineContext& context);

 private:
  static void printTiming(double ms);
};

}  // namespace core::pipeline

#endif  // APPLICATION_PIPELINE_STEPS_CONVERTER_STEP_H_
