// core/pipeline/steps/source_validator_step.hpp
#ifndef CORE_PIPELINE_STEPS_SOURCE_VALIDATOR_STEP_HPP_
#define CORE_PIPELINE_STEPS_SOURCE_VALIDATOR_STEP_HPP_

#include "core/pipeline/context/pipeline_context.hpp"

namespace core::pipeline {

class SourceValidatorStep {
public:
    bool execute(PipelineContext& context);

private:
    void printTiming(double ms) const;
};

} // namespace core::pipeline

#endif // CORE_PIPELINE_STEPS_SOURCE_VALIDATOR_STEP_HPP_