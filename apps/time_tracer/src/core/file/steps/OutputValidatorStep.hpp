// core/file/steps/OutputValidatorStep.hpp
#ifndef CORE_FILE_STEPS_OUTPUT_VALIDATOR_STEP_HPP_
#define CORE_FILE_STEPS_OUTPUT_VALIDATOR_STEP_HPP_

#include "core/file/PipelineContext.hpp"

class OutputValidatorStep {
public:
    bool execute(PipelineContext& context);

private:
    void printTiming(double ms) const;
};

#endif // CORE_FILE_STEPS_OUTPUT_VALIDATOR_STEP_HPP_