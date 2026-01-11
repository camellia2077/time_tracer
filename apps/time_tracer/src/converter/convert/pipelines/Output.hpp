// converter/convert/pipelines/Output.hpp
#ifndef OUTPUT_HPP
#define OUTPUT_HPP

#include <ostream>
#include <vector> 
#include <nlohmann/json.hpp>
#include "converter/convert/model/InputData.hpp"
#include "converter/convert/config/ConverterConfig.hpp"

// 用于管理json的键的命名

class Output {
public:
    void write(std::ostream& outputStream, const std::vector<InputData>& days, const ConverterConfig& config);
};

#endif // OUTPUT_HPP