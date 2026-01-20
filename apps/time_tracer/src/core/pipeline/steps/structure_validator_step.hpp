// core/pipeline/steps/structure_validator_step.hpp
#ifndef CORE_PIPELINE_STEPS_STRUCTURE_VALIDATOR_STEP_HPP_
#define CORE_PIPELINE_STEPS_STRUCTURE_VALIDATOR_STEP_HPP_

#include "core/pipeline/context/pipeline_context.hpp"

namespace core::pipeline {

class StructureValidatorStep {
public:
    bool execute(PipelineContext& context);
};

} 
#endif