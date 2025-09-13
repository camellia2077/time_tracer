// reprocessing/validator/source_txt/pipelines/SourceValidatorConfig.hpp
#ifndef SOURCE_VALIDATOR_CONFIG_HPP
#define SOURCE_VALIDATOR_CONFIG_HPP

#include <string>
#include <unordered_set>
#include <nlohmann/json.hpp>

// 这个类现在专门负责加载和存储验证所需的配置

// 负责加载和持有验证规则的配置
class SourceValidatorConfig {
public:
    explicit SourceValidatorConfig(const std::string& config_filename);

    const std::string& get_remark_prefix() const;
    const std::unordered_set<std::string>& get_valid_event_keywords() const;
    const std::unordered_set<std::string>& get_wake_keywords() const;

private:
    void _load(const std::string& config_filename);
    
    std::string remark_prefix_;
    std::unordered_set<std::string> valid_event_keywords_;
    std::unordered_set<std::string> wake_keywords_;
};

#endif // SOURCE_VALIDATOR_CONFIG_HPP