// core/file/steps/ConverterStep.hpp
#ifndef CORE_FILE_STEPS_CONVERTER_STEP_HPP_
#define CORE_FILE_STEPS_CONVERTER_STEP_HPP_

#include "core/file/PipelineContext.hpp"
#include "converter/LogProcessor.hpp"

class ConverterStep {
public:
    explicit ConverterStep(const AppConfig& config);
    bool execute(PipelineContext& context);

private:
    const AppConfig& app_config_; // [修改] 只持有 AppConfig
    void printTiming(double ms) const;
};

#endif // CORE_FILE_STEPS_CONVERTER_STEP_HPP_
