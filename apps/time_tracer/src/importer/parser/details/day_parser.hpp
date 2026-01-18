// importer/parser/details/day_parser.hpp
#ifndef IMPORTER_PARSER_DETAILS_DAY_PARSER_HPP_
#define IMPORTER_PARSER_DETAILS_DAY_PARSER_HPP_

#include "importer/model/time_sheet_data.hpp"
#include <nlohmann/json.hpp>

class DayParser {
public:
    DayData parse(const nlohmann::json& day_json) const;
};

#endif // IMPORTER_PARSER_DETAILS_DAY_PARSER_HPP_