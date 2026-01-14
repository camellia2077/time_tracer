// converter/config/JsonConverterConfigLoader.hpp
#ifndef JSON_CONVERTER_CONFIG_LOADER_HPP
#define JSON_CONVERTER_CONFIG_LOADER_HPP

#include "common/config/IConfigLoader.hpp"
#include "converter/config/ConverterConfig.hpp"
#include <nlohmann/json.hpp>

/**
 * @brief 基于 JSON 的 Converter 配置加载器
 * 实现 IConfigLoader 接口，负责将 JSON 数据解析并填充到 ConverterConfig 中。
 */
class JsonConverterConfigLoader : public IConfigLoader<ConverterConfig> {
public:
    /**
     * @brief 构造函数
     * @param config_json 已经加载或合并好的 JSON 对象
     */
    explicit JsonConverterConfigLoader(const nlohmann::json& config_json);

    bool load(ConverterConfig& config_object) override;

private:
    const nlohmann::json& json_source_;
};

#endif // JSON_CONVERTER_CONFIG_LOADER_HPP