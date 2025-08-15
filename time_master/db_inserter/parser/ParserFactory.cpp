
#include "ParserFactory.h"
#include "internal/ConfigLoader.h" // 工厂内部依赖ConfigLoader来加载配置
#include "internal/ParserConfig.h" // 工厂内部需要知道配置的数据结构

/**
 * @brief 实现创建解析器的具体逻辑。
 * @details 此方法首先调用 ConfigLoader 加载配置，
 * 然后使用加载到的配置对象来构造并返回一个 DataFileParser 实例。
 */
DataFileParser ParserFactory::create_parser(const std::string& config_path) {
    // 步骤1：调用ConfigLoader加载配置，这是ConfigLoader的职责。
    ParserConfig config = ConfigLoader::load_from_file(config_path);

    // 步骤2：使用加载好的配置创建解析器实例，并返回。
    // 将这两个步骤完全封装在工厂内部。
    return DataFileParser(config);
}