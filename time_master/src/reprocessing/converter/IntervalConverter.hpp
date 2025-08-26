// reprocessing/converter/IntervalConverter.hpp
#ifndef INTERVAL_PROCESSOR_HPP
#define INTERVAL_PROCESSOR_HPP

#include <string>
#include "reprocessing/converter/internal/ConverterConfig.hpp"

class IntervalConverter {
public:
    explicit IntervalConverter(const std::string& config_filename);

    bool executeConversion(const std::string& input_filepath, const std::string& output_filepath, const std::string& year_prefix);

private:
    ConverterConfig config_;
};

#endif // INTERVAL_PROCESSOR_HPP