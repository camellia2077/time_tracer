// converter/internal/OutputGenerator.h
#ifndef OUTPUT_GENERATOR_H
#define OUTPUT_GENERATOR_H
// 此类将负责格式化并输出内容
#include <ostream>
#include "reprocessing/converter/model/InputData.h"
#include "reprocessing/converter/internal/ConverterConfig.h"

class OutputGenerator {
public:
    void write(std::ostream& outputStream, const InputData& day, const ConverterConfig& config);
};

#endif // OUTPUT_GENERATOR_H