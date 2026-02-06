// application/pipeline/steps/structure_validator_step.hpp
#ifndef APPLICATION_PIPELINE_STEPS_STRUCTURE_VALIDATOR_STEP_H_
#define APPLICATION_PIPELINE_STEPS_STRUCTURE_VALIDATOR_STEP_H_

#include "application/pipeline/context/pipeline_context.hpp"

namespace core::pipeline {

class StructureValidatorStep {
 public:
  static auto Execute(PipelineContext& context) -> bool;
};

}  // namespace core::pipeline
#endif
