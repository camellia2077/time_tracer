#ifndef PARSER_FACTORY_H
#define PARSER_FACTORY_H

#include <string>
#include "internal/DataFileParser.h"

/**
 * @class ParserFactory
 * @brief 负责创建和配置 DataFileParser 实例的工厂类。
 * @details 封装了从文件加载配置并用该配置初始化解析器的所有细节，
 * 使客户端代码与配置加载和解析器构造过程解耦。
 */
class ParserFactory {
public:
    /**
     * @brief 根据配置文件路径创建一个功能完备的 DataFileParser 实例。
     * @param config_path JSON 配置文件的路径。
     * @return 一个已配置好的 DataFileParser 对象。
     */
    static DataFileParser create_parser(const std::string& config_path);
};

#endif // PARSER_FACTORY_H