// importer/parser/details/activity_parser.hpp
#ifndef IMPORTER_PARSER_DETAILS_ACTIVITY_PARSER_HPP_
#define IMPORTER_PARSER_DETAILS_ACTIVITY_PARSER_HPP_

#include "importer/model/time_sheet_data.hpp"
#include <nlohmann/json.hpp>

class ActivityParser {
public:
    TimeRecordInternal parse(
        const nlohmann::json& activity_json,
        const std::string& date) const;
};

#endif // IMPORTER_PARSER_DETAILS_ACTIVITY_PARSER_HPP_