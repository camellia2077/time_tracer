// importer/parser/details/DayParser.hpp
#ifndef DAY_PARSER_HPP
#define DAY_PARSER_HPP

#include "importer/model/TimeSheetData.hpp"
#include <nlohmann/json.hpp>

class DayParser {
public:
    DayData parse(const nlohmann::json& day_json) const;
};

#endif // DAY_PARSER_HPP