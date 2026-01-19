// converter/convert/io/text_parser.hpp
#ifndef CONVERTER_CONVERT_IO_TEXT_PARSER_HPP_
#define CONVERTER_CONVERT_IO_TEXT_PARSER_HPP_

#include <string>
#include <functional>
#include <iostream>
#include <unordered_set>
#include "common/model/daily_log.hpp"
#include "common/config/models/converter_config_models.hpp"


class TextParser {
public:
    explicit TextParser(const ConverterConfig& config);
    void parse(std::istream& inputStream, std::function<void(DailyLog&)> onNewDay);

private:
    const ConverterConfig& config_;
    
    std::string year_prefix_; 
    const std::vector<std::string>& wake_keywords_; // [优化] 直接引用 vector，避免拷贝 set

    bool isYearMarker(const std::string& line) const;
    bool isNewDayMarker(const std::string& line) const;
    void parseLine(const std::string& line, DailyLog& currentDay) const;
};

#endif // CONVERTER_CONVERT_IO_TEXT_PARSER_HPP_