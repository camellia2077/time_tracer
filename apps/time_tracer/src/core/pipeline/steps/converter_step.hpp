// core/pipeline/steps/converter_step.hpp
#ifndef CORE_PIPELINE_STEPS_CONVERTER_STEP_HPP_
#define CORE_PIPELINE_STEPS_CONVERTER_STEP_HPP_

#include "core/pipeline/context/pipeline_context.hpp"
#include "common/config/app_config.hpp"

namespace core::pipeline {

class ConverterStep {
public:
    explicit ConverterStep(const AppConfig& config);
    bool execute(PipelineContext& context);

private:
    const AppConfig& app_config_;
    void printTiming(double ms) const;
};

} // namespace core::pipeline

#endif // CORE_PIPELINE_STEPS_CONVERTER_STEP_HPP_