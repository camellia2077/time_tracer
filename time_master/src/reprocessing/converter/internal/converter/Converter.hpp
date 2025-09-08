// reprocessing/converter/internal/converter/Converter.hpp
#ifndef INTERVAL_CONVERTER_HPP
#define INTERVAL_CONVERTER_HPP

#include "reprocessing/converter/model/InputData.hpp"
#include "reprocessing/converter/internal/ConverterConfig.hpp"
#include <string>

// [修改] 移除不再需要的成员变量
class Converter {
public:
    explicit Converter(const ConverterConfig& config);

    void transform(InputData& day);

private:
    const ConverterConfig& config_;
};

#endif // INTERVAL_CONVERTER_HPP