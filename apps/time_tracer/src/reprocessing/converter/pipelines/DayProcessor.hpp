// reprocessing/converter/pipelines/DayProcessor.hpp
#ifndef DAY_PROCESSOR_HPP
#define DAY_PROCESSOR_HPP

// 这个类将负责处理天与天之间的关联逻辑

#include "reprocessing/converter/model/InputData.hpp"
#include "reprocessing/converter/config/ConverterConfig.hpp" // <-- [FIX] Added missing include

class DayProcessor {
public:
    // --- [核心修改] 构造函数现在接收 ConverterConfig 的引用 ---
    explicit DayProcessor(const ConverterConfig& config);

    void process(InputData& previousDay, InputData& dayToProcess);

private:
    // --- [核心修改] 成员变量变更为 config_ ---
    const ConverterConfig& config_;
};

#endif // DAY_PROCESSOR_HPP