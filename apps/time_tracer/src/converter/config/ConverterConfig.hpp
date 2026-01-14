// converter/config/ConverterConfig.hpp
#ifndef CONVERTER_CONFIG_HPP
#define CONVERTER_CONFIG_HPP

#include <string>
#include <vector>
#include <unordered_map>

// 保持 DurationRule 结构定义
struct DurationRule {
    int less_than_minutes;
    std::string value;
};

class ConverterConfig {
public:
    // [架构优化] 移除 load 方法和 json 依赖
    // bool load(const nlohmann::json& config_json); <--- 已移除

    // Getters (保持不变)
    const std::string& getRemarkPrefix() const;
    const std::vector<std::string>& getHeaderOrder() const;
    const std::unordered_map<std::string, std::string>& getTextMapping() const;
    const std::unordered_map<std::string, std::string>& getTextDurationMapping() const;
    const std::unordered_map<std::string, std::vector<DurationRule>>& getDurationMappings() const;
    const std::vector<std::string>& getWakeKeywords() const;
    const std::unordered_map<std::string, std::string>& getTopParentMapping() const;

    // Public member (保持不变)
    std::unordered_map<std::string, std::string> initial_top_parents;

    // [新增] Setters (供 Loader 使用)
    void setRemarkPrefix(const std::string& prefix);
    void setHeaderOrder(const std::vector<std::string>& order);
    void setTextMapping(const std::unordered_map<std::string, std::string>& mapping);
    void setTextDurationMapping(const std::unordered_map<std::string, std::string>& mapping);
    void setDurationMappings(const std::unordered_map<std::string, std::vector<DurationRule>>& mappings);
    void setWakeKeywords(const std::vector<std::string>& keywords);
    void setTopParentMapping(const std::unordered_map<std::string, std::string>& mapping);

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