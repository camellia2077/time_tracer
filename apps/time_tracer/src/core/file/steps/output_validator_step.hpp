// core/file/steps/output_validator_step.hpp
#ifndef CORE_FILE_STEPS_OUTPUT_VALIDATOR_STEP_HPP_
#define CORE_FILE_STEPS_OUTPUT_VALIDATOR_STEP_HPP_

#include "core/file/pipeline_context.hpp"

class OutputValidatorStep {
public:
    bool execute(PipelineContext& context);

private:
    void printTiming(double ms) const;
};

#endif // CORE_FILE_STEPS_OUTPUT_VALIDATOR_STEP_HPP_