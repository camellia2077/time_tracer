// converter/internal/InputParser.hpp
#ifndef INPUT_PARSER_HPP
#define INPUT_PARSER_HPP

// 这个类将包含原来 IntervalConverter 中的行解析逻辑
#include <string>
#include <functional>
#include <iostream>
#include "reprocessing/converter/model/InputData.hpp"
#include "reprocessing/converter/internal/ConverterConfig.hpp"

class InputParser {
public:
    InputParser(const ConverterConfig& config, const std::string& year_prefix);
    void parse(std::istream& inputStream, std::function<void(InputData&)> onNewDay);

private:
    const ConverterConfig& config_;
    const std::string year_prefix_;

    bool isNewDayMarker(const std::string& line) const;
    void parseLine(const std::string& line, InputData& currentDay) const;
};

#endif // INPUT_PARSER_HPP