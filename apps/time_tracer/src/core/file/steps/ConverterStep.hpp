// core/file/steps/ConverterStep.hpp
#ifndef CONVERTER_STEP_HPP
#define CONVERTER_STEP_HPP

#include "core/file/PipelineContext.hpp"
#include "reprocessing/LogProcessor.hpp"

class ConverterStep {
public:
    explicit ConverterStep(const AppConfig& config);
    bool execute(PipelineContext& context);

private:
    LogProcessor processor_; 
    void printTiming(double ms) const;
};

#endif // CONVERTER_STEP_HPP