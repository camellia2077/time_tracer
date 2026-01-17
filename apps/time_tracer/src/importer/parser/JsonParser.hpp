// importer/parser/JsonParser.hpp
#ifndef IMPORTER_PARSER_JSON_PARSER_HPP_
#define IMPORTER_PARSER_JSON_PARSER_HPP_

#include <string>
#include "importer/parser/ParsedData.hpp"

class JsonParser {
public:
    JsonParser() = default;

    /**
     * @brief 解析 JSON 字符串内容为 ParsedData 对象。
     * @param json_content 完整的 JSON 字符串。
     * @param context_name 用于错误日志的上下文标识（例如文件名），默认为 "memory_content"。
     * @return 解析后的 ParsedData 结构。
     * @throws std::runtime_error 如果解析失败。
     */
    ParsedData parse_content(const std::string& json_content, const std::string& context_name = "memory_content");
};

#endif // IMPORTER_PARSER_JSON_PARSER_HPP_