// reprocessing/input_transfer/internal/ConverterConfig.hpp

#ifndef INTERVAL_PROCESSOR_CONFIG_HPP
#define INTERVAL_PROCESSOR_CONFIG_HPP

#include <string>
#include <vector>
#include <unordered_map>

struct DurationRule {
    int less_than_minutes;
    std::string value;
};

class ConverterConfig {
public:
    bool load(const std::string& filepath);

    const std::string& getRemarkPrefix() const;
    const std::vector<std::string>& getHeaderOrder() const;
    const std::unordered_map<std::string, std::string>& getTextMapping() const;
    const std::unordered_map<std::string, std::string>& getTextDurationMapping() const;
    const std::unordered_map<std::string, std::vector<DurationRule>>& getDurationMappings() const;
    const std::vector<std::string>& getWakeKeywords() const;

    // [新增] Getter for initial_top_parents mapping
    const std::unordered_map<std::string, std::string>& getInitialTopParentsMapping() const;

    // [新增] Setter for initial_top_parents mapping
    void setInitialTopParentsMapping(const std::unordered_map<std::string, std::string>& mapping);

private:
    std::string remark_prefix_;
    std::vector<std::string> header_order_;
    std::unordered_map<std::string, std::string> text_mapping_;
    std::unordered_map<std::string, std::string> text_duration_mapping_;
    std::unordered_map<std::string, std::vector<DurationRule>> duration_mappings_;
    std::vector<std::string> wake_keywords_;

    // [新增] Member to store the initial_top_parents mapping
    std::unordered_map<std::string, std::string> initial_top_parents_mapping_;
};

#endif // INTERVAL_PROCESSOR_CONFIG_HPP