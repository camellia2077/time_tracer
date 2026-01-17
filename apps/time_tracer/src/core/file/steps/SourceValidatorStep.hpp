// core/file/steps/SourceValidatorStep.hpp
#ifndef CORE_FILE_STEPS_SOURCE_VALIDATOR_STEP_HPP_
#define CORE_FILE_STEPS_SOURCE_VALIDATOR_STEP_HPP_

#include "core/file/PipelineContext.hpp"
#include "converter/LogProcessor.hpp"

class SourceValidatorStep {
public:
    bool execute(PipelineContext& context);

private:
    void printTiming(double ms) const;
};

#endif // CORE_FILE_STEPS_SOURCE_VALIDATOR_STEP_HPP_