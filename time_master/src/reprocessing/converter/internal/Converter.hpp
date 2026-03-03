// reprocessing/input_transfer/internal/Converter.hpp
#ifndef INTERVAL_CONVERTER_HPP
#define INTERVAL_CONVERTER_HPP

#include "reprocessing/converter/model/InputData.hpp"
#include "ConverterConfig.hpp"

class Converter {
public:
    explicit Converter(const ConverterConfig& config);

    void transform(InputData& day);

private:
    const ConverterConfig& config_;
};

#endif // INTERVAL_CONVERTER_HPP