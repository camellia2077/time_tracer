// db_inserter/parser/facade/MemoryParserFacade.hpp
#ifndef MEMORY_PARSER_FACADE_HPP
#define MEMORY_PARSER_FACADE_HPP

#include <map>
#include <string>
#include <vector>
#include "db_inserter/parser/model/ParsedData.hpp"
#include "reprocessing/converter/model/InputData.hpp"

/**
 * @class MemoryParserFacade
 * @brief 负责将内存中的转换层数据模型 (InputData) 转换为 数据库层数据模型 (ParsedData)。
 * 充当 reprocessing 模块与 db_inserter 模块之间的适配器。
 */
class MemoryParserFacade {
public:
    MemoryParserFacade() = default;

    /**
     * @brief 执行转换。
     * @param data_map 内存中的按月分组数据。
     * @return 可直接用于入库的 ParsedData 结构。
     */
    ParsedData parse(const std::map<std::string, std::vector<InputData>>& data_map);
};

#endif // MEMORY_PARSER_FACADE_HPP