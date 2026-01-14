// converter/config/ConverterConfig.cpp
#include "ConverterConfig.hpp"

// [架构优化] 移除 JSON 解析逻辑，只保留纯数据的 Get/Set

const std::string& ConverterConfig::getRemarkPrefix() const { return remark_prefix_; }
const std::vector<std::string>& ConverterConfig::getHeaderOrder() const { return header_order_; }
const std::unordered_map<std::string, std::string>& ConverterConfig::getTextMapping() const { return text_mapping_; }
const std::unordered_map<std::string, std::string>& ConverterConfig::getTextDurationMapping() const { return text_duration_mapping_; }
const std::unordered_map<std::string, std::vector<DurationRule>>& ConverterConfig::getDurationMappings() const { return duration_mappings_; }
const std::vector<std::string>& ConverterConfig::getWakeKeywords() const { return wake_keywords_; }
const std::unordered_map<std::string, std::string>& ConverterConfig::getTopParentMapping() const { return topParentMapping_; }

// Setters Implementation
void ConverterConfig::setRemarkPrefix(const std::string& prefix) { remark_prefix_ = prefix; }
void ConverterConfig::setHeaderOrder(const std::vector<std::string>& order) { header_order_ = order; }
void ConverterConfig::setTextMapping(const std::unordered_map<std::string, std::string>& mapping) { text_mapping_ = mapping; }
void ConverterConfig::setTextDurationMapping(const std::unordered_map<std::string, std::string>& mapping) { text_duration_mapping_ = mapping; }
void ConverterConfig::setDurationMappings(const std::unordered_map<std::string, std::vector<DurationRule>>& mappings) { duration_mappings_ = mappings; }
void ConverterConfig::setWakeKeywords(const std::vector<std::string>& keywords) { wake_keywords_ = keywords; }
void ConverterConfig::setTopParentMapping(const std::unordered_map<std::string, std::string>& mapping) { topParentMapping_ = mapping; }