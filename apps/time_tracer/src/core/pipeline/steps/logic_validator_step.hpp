// core/pipeline/steps/logic_validator_step.hpp
#ifndef CORE_PIPELINE_STEPS_LOGIC_VALIDATOR_STEP_HPP_
#define CORE_PIPELINE_STEPS_LOGIC_VALIDATOR_STEP_HPP_

#include "core/pipeline/context/pipeline_context.hpp"

namespace core::pipeline {

class LogicValidatorStep {
public:
    bool execute(PipelineContext& context);
};

} 
#endif