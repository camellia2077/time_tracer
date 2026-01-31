// application/pipeline/steps/logic_validator_step.hpp
#ifndef APPLICATION_PIPELINE_STEPS_LOGIC_VALIDATOR_STEP_H_
#define APPLICATION_PIPELINE_STEPS_LOGIC_VALIDATOR_STEP_H_

#include "application/pipeline/context/pipeline_context.hpp"

namespace core::pipeline {

class LogicValidatorStep {
 public:
  static bool execute(PipelineContext& context);
};

}  // namespace core::pipeline
#endif
