// reprocessing/converter/internal/OutputGenerator.hpp
#ifndef OUTPUT_GENERATOR_HPP
#define OUTPUT_GENERATOR_HPP

#include <ostream>
#include <vector> 
#include <nlohmann/json.hpp>
#include "reprocessing/converter/model/InputData.hpp"
#include "reprocessing/converter/internal/ConverterConfig.hpp"

class OutputGenerator {
public:
    void write(std::ostream& outputStream, const std::vector<InputData>& days, const ConverterConfig& config);
};

#endif // OUTPUT_GENERATOR_HPP