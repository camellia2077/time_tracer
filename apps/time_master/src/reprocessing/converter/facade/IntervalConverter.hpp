// reprocessing/converter/facade/IntervalConverter.hpp
#ifndef INTERVAL_CONVERTER_HPP
#define INTERVAL_CONVERTER_HPP

#include "reprocessing/converter/config/ConverterConfig.hpp"
#include "reprocessing/converter/model/InputData.hpp" /
#include <string>
#include <istream>
#include <vector> 

class IntervalConverter {
public:
    explicit IntervalConverter(const std::string& config_filename);

    std::vector<InputData> executeConversion(std::istream& combined_input_stream);

private:
    ConverterConfig config_;
};

#endif // INTERVAL_CONVERTER_HPP