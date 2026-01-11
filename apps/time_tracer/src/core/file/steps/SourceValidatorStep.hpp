// core/file/steps/SourceValidatorStep.hpp
#ifndef SOURCE_VALIDATOR_STEP_HPP
#define SOURCE_VALIDATOR_STEP_HPP

#include "core/file/PipelineContext.hpp"
#include "converter/LogProcessor.hpp"

class SourceValidatorStep {
public:
    bool execute(PipelineContext& context);

private:
    void printTiming(double ms) const;
};

#endif // SOURCE_VALIDATOR_STEP_HPP