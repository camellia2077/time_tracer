// reprocessing/converter/facade/IntervalConverter.hpp
#ifndef INTERVAL_PROCESSOR_HPP
#define INTERVAL_PROCESSOR_HPP

#include "reprocessing/converter/config/ConverterConfig.hpp"
#include "reprocessing/converter/model/InputData.hpp" // [核心修改] 引入 InputData
#include <string>
#include <istream>
#include <vector> // [核心修改] 引入 vector

class IntervalConverter {
public:
    explicit IntervalConverter(const std::string& config_filename);

    std::vector<InputData> executeConversion(std::istream& combined_input_stream);

private:
    ConverterConfig config_;
};

#endif // INTERVAL_PROCESSOR_HPP