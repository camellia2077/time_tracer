// db_inserter/parser/facade/JsonParserFacade.hpp
#ifndef JSON_PARSER_FACADE_HPP
#define JSON_PARSER_FACADE_HPP

#include <string>
#include "db_inserter/parser/model/ParsedData.hpp"

// --- [FIX] Renamed class from JsonDataParser to JsonParserFacade ---
class JsonParserFacade {
public:
    JsonParserFacade() = default;

    // The main parsing function now returns a ParsedData object or throws an exception.
    ParsedData parse_file(const std::string& filename);
};

#endif // JSON_PARSER_FACADE_HPP