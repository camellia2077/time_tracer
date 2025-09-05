// reprocessing/converter/internal/InputParser.hpp
#ifndef INPUT_PARSER_HPP
#define INPUT_PARSER_HPP

#include <string>
#include <functional>
#include <iostream>
#include <unordered_set>
#include "reprocessing/converter/model/InputData.hpp"
#include "reprocessing/converter/internal/ConverterConfig.hpp"

class InputParser {
public:
    InputParser(const ConverterConfig& config, const std::string& year_prefix);
    void parse(std::istream& inputStream, std::function<void(InputData&)> onNewDay);

private:
    const ConverterConfig& config_;
    const std::string year_prefix_;
    // [优化] 将 unordered_set 作为成员变量，避免在 parseLine 中重复创建
    const std::unordered_set<std::string> wake_keywords_;

    bool isNewDayMarker(const std::string& line) const;
    void parseLine(const std::string& line, InputData& currentDay) const;
};

#endif // INPUT_PARSER_HPP