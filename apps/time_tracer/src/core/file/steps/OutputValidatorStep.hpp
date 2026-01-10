// core/file/steps/OutputValidatorStep.hpp
#ifndef OUTPUT_VALIDATOR_STEP_HPP
#define OUTPUT_VALIDATOR_STEP_HPP

#include "core/file/PipelineContext.hpp"

class OutputValidatorStep {
public:
    bool execute(PipelineContext& context);

private:
    void printTiming(double ms) const;
};

#endif // OUTPUT_VALIDATOR_STEP_HPP