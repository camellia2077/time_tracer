// converter/internal/DayProcessor.h
#ifndef DAY_PROCESSOR_H
#define DAY_PROCESSOR_H

// 这个类将负责处理天与天之间的关联逻辑

#include "reprocessing/converter/model/InputData.h"
#include "reprocessing/converter/internal/Converter.h"

class DayProcessor {
public:
    explicit DayProcessor(Converter& converter);
    void process(InputData& dayToFinalize, InputData& nextDay);

private:
    Converter& converter_;
};

#endif // DAY_PROCESSOR_H