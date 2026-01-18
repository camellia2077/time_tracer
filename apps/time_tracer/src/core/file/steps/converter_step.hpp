// core/file/steps/converter_step.hpp
#ifndef CORE_FILE_STEPS_CONVERTER_STEP_HPP_
#define CORE_FILE_STEPS_CONVERTER_STEP_HPP_

#include "core/file/pipeline_context.hpp"
#include "converter/log_processor.hpp"

class ConverterStep {
public:
    explicit ConverterStep(const AppConfig& config);
    bool execute(PipelineContext& context);

private:
    const AppConfig& app_config_; // [修改] 只持有 AppConfig
    void printTiming(double ms) const;
};

#endif // CORE_FILE_STEPS_CONVERTER_STEP_HPP_
