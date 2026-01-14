// converter/config/ConverterConfig.hpp

#ifndef CONVERTER_CONFIG_HPP
#define CONVERTER_CONFIG_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <nlohmann/json.hpp> 

struct DurationRule {
    int less_than_minutes;
    std::string value;
};

class ConverterConfig {
public:
    /**
     * @brief 从 JSON 对象加载配置。
     * @details 调用者负责将主配置和子配置（如 mappings）合并到一个 JSON 对象中传入。
     */
    bool load(const nlohmann::json& config_json);

    const std::string& getRemarkPrefix() const;
    const std::vector<std::string>& getHeaderOrder() const;
    const std::unordered_map<std::string, std::string>& getTextMapping() const;
    const std::unordered_map<std::string, std::string>& getTextDurationMapping() const;
    const std::unordered_map<std::string, std::vector<DurationRule>>& getDurationMappings() const;
    const std::vector<std::string>& getWakeKeywords() const;
    const std::unordered_map<std::string, std::string>& getTopParentMapping() const;

    // [修复] 新增公有成员，允许 Core 层注入外部定义的初始父节点映射
    // 使用 string 作为键值，因为 JSON 配置也是基于字符串的
    std::unordered_map<std::string, std::string> initial_top_parents;

private:
    std::string remark_prefix_;
    std::vector<std::string> header_order_;
    std::unordered_map<std::string, std::string> text_mapping_;
    std::unordered_map<std::string, std::string> text_duration_mapping_;
    std::unordered_map<std::string, std::vector<DurationRule>> duration_mappings_;
    std::vector<std::string> wake_keywords_;
    std::unordered_map<std::string, std::string> topParentMapping_;
};

#endif // CONVERTER_CONFIG_HPP