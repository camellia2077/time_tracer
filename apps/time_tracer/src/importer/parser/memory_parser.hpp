// importer/parser/memory_parser.hpp
#ifndef IMPORTER_PARSER_MEMORY_PARSER_HPP_
#define IMPORTER_PARSER_MEMORY_PARSER_HPP_

#include <map>
#include <string>
#include <vector>
#include "importer/parser/parsed_data.hpp"
#include "common/model/daily_log.hpp"

/**
 * @class MemoryParser
 * @brief 负责将内存中的转换层数据模型 (DailyLog) 转换为 数据库层数据模型 (ParsedData)。
 * 充当 reprocessing 模块与 db_inserter 模块之间的适配器。
 */
class MemoryParser {
public:
    MemoryParser() = default;

    /**
     * @brief 执行转换。
     * @param data_map 内存中的按月分组数据。
     * @return 可直接用于入库的 ParsedData 结构。
     */
    ParsedData parse(const std::map<std::string, std::vector<DailyLog>>& data_map);
};

#endif // IMPORTER_PARSER_MEMORY_PARSER_HPP_