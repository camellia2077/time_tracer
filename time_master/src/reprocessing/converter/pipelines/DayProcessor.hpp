// converter/internal/DayProcessor.hpp
#ifndef DAY_PROCESSOR_HPP
#define DAY_PROCESSOR_HPP

// 这个类将负责处理天与天之间的关联逻辑

#include "reprocessing/converter/model/InputData.hpp"
#include "reprocessing/converter/pipelines/converter/Converter.hpp"

class DayProcessor {
public:
    explicit DayProcessor(Converter& converter);
    // --- [核心修改] 更改函数签名以反映新的逻辑 ---
    void process(InputData& previousDay, InputData& dayToProcess);

private:
    Converter& converter_;
};

#endif // DAY_PROCESSOR_HPP