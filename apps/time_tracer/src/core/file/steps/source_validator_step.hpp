// core/file/steps/source_validator_step.hpp
#ifndef CORE_FILE_STEPS_SOURCE_VALIDATOR_STEP_HPP_
#define CORE_FILE_STEPS_SOURCE_VALIDATOR_STEP_HPP_

#include "core/file/pipeline_context.hpp"
#include "converter/log_processor.hpp"

class SourceValidatorStep {
public:
    bool execute(PipelineContext& context);

private:
    void printTiming(double ms) const;
};

#endif // CORE_FILE_STEPS_SOURCE_VALIDATOR_STEP_HPP_