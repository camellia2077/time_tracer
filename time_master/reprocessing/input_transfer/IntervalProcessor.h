// --- START OF FILE reprocessing/input_transfer/IntervalProcessor.h ---

#ifndef INTERVAL_PROCESSOR_H
#define INTERVAL_PROCESSOR_H

#include <string>
#include "internal/IntervalProcessorConfig.h" // 包含配置类

class IntervalProcessor {
public:
    explicit IntervalProcessor(const std::string& config_filename);

    bool executeConversion(const std::string& input_filepath, const std::string& output_filepath, const std::string& year_prefix);

private:
    IntervalProcessorConfig config_; // 将配置作为成员变量
};

#endif // INTERVAL_PROCESSOR_H