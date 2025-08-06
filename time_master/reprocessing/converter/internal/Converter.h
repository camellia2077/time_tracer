// reprocessing/input_transfer/internal/Converter.h
#ifndef INTERVAL_CONVERTER_H
#define INTERVAL_CONVERTER_H

#include "reprocessing/converter/model/InputData.h"
#include "ConverterConfig.h"

class Converter {
public:
    explicit Converter(const ConverterConfig& config);

    void transform(InputData& day);

private:
    const ConverterConfig& config_;
};

#endif // INTERVAL_CONVERTER_H