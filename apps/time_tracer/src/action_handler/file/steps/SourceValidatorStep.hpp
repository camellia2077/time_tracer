// action_handler/file/steps/SourceValidatorStep.hpp
#ifndef SOURCE_VALIDATOR_STEP_HPP
#define SOURCE_VALIDATOR_STEP_HPP

#include "action_handler/file/PipelineContext.hpp"
#include "reprocessing/LogProcessor.hpp"

class SourceValidatorStep {
public:
    bool execute(PipelineContext& context);

private:
    void printTiming(double ms) const;
};

#endif // SOURCE_VALIDATOR_STEP_HPP