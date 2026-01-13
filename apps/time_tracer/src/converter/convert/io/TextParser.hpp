// converter/convert/io/TextParser.hpp
#ifndef TEXT_PARSER_HPP
#define TEXT_PARSER_HPP

#include <string>
#include <functional>
#include <iostream>
#include <unordered_set>
#include "common/model/DailyLog.hpp"
#include "converter/config/ConverterConfig.hpp"


class TextParser {
public:
    // [修改] 构造函数不再需要 year_prefix
    explicit TextParser(const ConverterConfig& config);
    void parse(std::istream& inputStream, std::function<void(DailyLog&)> onNewDay);

private:
    const ConverterConfig& config_;
    
    std::string year_prefix_; // [修改] year_prefix_ 现在由 parse 方法在内部确定
    const std::unordered_set<std::string> wake_keywords_;

    
    bool isYearMarker(const std::string& line) const;// [新增] 用于识别年份行的辅助函数
    bool isNewDayMarker(const std::string& line) const;
    void parseLine(const std::string& line, DailyLog& currentDay) const;
};

#endif // TEXT_PARSER_HPP