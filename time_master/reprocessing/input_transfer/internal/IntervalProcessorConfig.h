// reprocessing/input_transfer/internal/IntervalProcessorConfig.cpp

#ifndef INTERVAL_PROCESSOR_CONFIG_H
#define INTERVAL_PROCESSOR_CONFIG_H

#include <string>
#include <vector>
#include <unordered_map>

struct DurationRule {
    int less_than_minutes;
    std::string value;
};

class IntervalProcessorConfig {
public:
    bool load(const std::string& filepath);

    const std::string& getRemarkPrefix() const;
    const std::vector<std::string>& getHeaderOrder() const;
    const std::unordered_map<std::string, std::string>& getTextMapping() const;
    const std::unordered_map<std::string, std::string>& getTextDurationMapping() const;
    const std::unordered_map<std::string, std::vector<DurationRule>>& getDurationMappings() const;

private:
    std::string remark_prefix_;
    std::vector<std::string> header_order_;
    std::unordered_map<std::string, std::string> text_mapping_;
    std::unordered_map<std::string, std::string> text_duration_mapping_;
    std::unordered_map<std::string, std::vector<DurationRule>> duration_mappings_;
};

#endif // INTERVAL_PROCESSOR_CONFIG_H