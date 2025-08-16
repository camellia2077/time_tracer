// reprocessing/converter/IntervalConverter.h
#ifndef INTERVAL_PROCESSOR_H
#define INTERVAL_PROCESSOR_H

#include <string>
#include "reprocessing/converter/internal/ConverterConfig.h"

class IntervalConverter {
public:
    explicit IntervalConverter(const std::string& config_filename);

    bool executeConversion(const std::string& input_filepath, const std::string& output_filepath, const std::string& year_prefix);

private:
    ConverterConfig config_;
};

#endif // INTERVAL_PROCESSOR_H