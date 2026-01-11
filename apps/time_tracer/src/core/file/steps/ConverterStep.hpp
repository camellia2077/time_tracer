// core/file/steps/ConverterStep.hpp
#ifndef CONVERTER_STEP_HPP
#define CONVERTER_STEP_HPP

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

#endif // CONVERTER_STEP_HPP
