// converter/config/converter_config.hpp
#ifndef CONVERTER_CONFIG_CONVERTER_CONFIG_HPP_
#define CONVERTER_CONFIG_CONVERTER_CONFIG_HPP_

#include <string>
#include <vector>
#include <unordered_map>

struct DurationMappingRule {
    int less_than_minutes;
    std::string value;
};

class ConverterConfig {
public:
    // Getters
    const std::string& getRemarkPrefix() const;
    const std::vector<std::string>& getHeaderOrder() const;
    const std::unordered_map<std::string, std::string>& getTextMapping() const;
    const std::unordered_map<std::string, std::string>& getTextDurationMapping() const;
    // [Fix] Updated type usage
    const std::unordered_map<std::string, std::vector<DurationMappingRule>>& getDurationMappings() const;
    const std::vector<std::string>& getWakeKeywords() const;
    const std::unordered_map<std::string, std::string>& getTopParentMapping() const;

    // Public member
    std::unordered_map<std::string, std::string> initial_top_parents;

    // Setters
    void setRemarkPrefix(const std::string& prefix);
    void setHeaderOrder(const std::vector<std::string>& order);
    void setTextMapping(const std::unordered_map<std::string, std::string>& mapping);
    void setTextDurationMapping(const std::unordered_map<std::string, std::string>& mapping);
    // [Fix] Updated type usage
    void setDurationMappings(const std::unordered_map<std::string, std::vector<DurationMappingRule>>& mappings);
    void setWakeKeywords(const std::vector<std::string>& keywords);
    void setTopParentMapping(const std::unordered_map<std::string, std::string>& mapping);

private:
    std::string remark_prefix_;
    std::vector<std::string> header_order_;
    std::unordered_map<std::string, std::string> text_mapping_;
    std::unordered_map<std::string, std::string> text_duration_mapping_;
    // [Fix] Updated type usage
    std::unordered_map<std::string, std::vector<DurationMappingRule>> duration_mappings_;
    std::vector<std::string> wake_keywords_;
    std::unordered_map<std::string, std::string> topParentMapping_;
};

#endif // CONVERTER_CONFIG_CONVERTER_CONFIG_HPP_