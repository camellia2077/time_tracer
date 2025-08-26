// converter/internal/OutputGenerator.hpp
#ifndef OUTPUT_GENERATOR_HPP
#define OUTPUT_GENERATOR_HPP
// 此类将负责格式化并输出内容
#include <ostream>
#include "reprocessing/converter/model/InputData.hpp"
#include "reprocessing/converter/internal/ConverterConfig.hpp"

class OutputGenerator {
public:
    void write(std::ostream& outputStream, const InputData& day, const ConverterConfig& config);
};

#endif // OUTPUT_GENERATOR_HPP