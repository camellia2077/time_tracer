// converter/internal/DayProcessor.hpp
#ifndef DAY_PROCESSOR_HPP
#define DAY_PROCESSOR_HPP

// 这个类将负责处理天与天之间的关联逻辑

#include "reprocessing/converter/model/InputData.hpp"
#include "reprocessing/converter/internal/converter/Converter.hpp"

class DayProcessor {
public:
    explicit DayProcessor(Converter& converter);
    void process(InputData& dayToFinalize, InputData& nextDay);

private:
    Converter& converter_;
};

#endif // DAY_PROCESSOR_HPP