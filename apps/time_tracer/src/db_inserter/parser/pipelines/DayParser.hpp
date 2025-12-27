// db_inserter/parser/pipelines/DayParser.hpp
#ifndef DAY_PARSER_HPP
#define DAY_PARSER_HPP

#include "db_inserter/model/time_sheet_model.hpp"
#include <nlohmann/json.hpp>

class DayParser {
public:
    DayData parse(const nlohmann::json& day_json) const;
};

#endif // DAY_PARSER_HPP