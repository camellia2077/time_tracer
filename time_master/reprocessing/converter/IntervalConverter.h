// reprocessing/converter/IntervalConverter.h

#ifndef INTERVAL_PROCESSOR_H
#define INTERVAL_PROCESSOR_H

#include <string>
#include <unordered_set>
#include "reprocessing/converter/internal/ConverterConfig.h"
#include "reprocessing/converter/model/InputData.h"

class IntervalConverter {
public:
    explicit IntervalConverter(const std::string& config_filename);

    bool executeConversion(const std::string& input_filepath, const std::string& output_filepath, const std::string& year_prefix);

private:
    // 将配置和关键词设为成员变量
    ConverterConfig config_;
    const std::unordered_set<std::string> wake_keywords_;

    // 私有辅助方法，用于封装逻辑
    bool isNewDayMarker(const std::string& line) const;
    void parseLine(const std::string& line, InputData& currentDay) const;
};

#endif // INTERVAL_PROCESSOR_H