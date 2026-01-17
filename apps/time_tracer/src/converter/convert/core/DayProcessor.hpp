// converter/convert/core/DayProcessor.hpp
#ifndef CONVERTER_CONVERT_CORE_DAY_PROCESSOR_HPP_
#define CONVERTER_CONVERT_CORE_DAY_PROCESSOR_HPP_

// 这个类将负责处理天与天之间的关联逻辑

#include "common/model/DailyLog.hpp"
#include "converter/config/ConverterConfig.hpp"
 // <-- [FIX] Added missing include

class DayProcessor {
public:
    // --- [核心修改] 构造函数现在接收 ConverterConfig 的引用 ---
    explicit DayProcessor(const ConverterConfig& config);

    void process(DailyLog& previousDay, DailyLog& dayToProcess);

private:
    // --- [核心修改] 成员变量变更为 config_ ---
    const ConverterConfig& config_;
};

#endif // CONVERTER_CONVERT_CORE_DAY_PROCESSOR_HPP_