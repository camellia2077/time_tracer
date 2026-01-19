// validator/txt/rules/LineRules.hpp
#ifndef VALIDATOR_TXT_RULES_LINE_RULES_HPP_
#define VALIDATOR_TXT_RULES_LINE_RULES_HPP_

// [Fix] 修改头文件路径：指向重构后的位置
#include "common/config/models/converter_config_models.hpp"
#include "validator/common/ValidatorUtils.hpp"
#include <string>
#include <set>
#include <unordered_set>

namespace validator {
namespace txt {

class LineRules {
public:
    explicit LineRules(const ConverterConfig& config);
    
    bool is_year(const std::string& line) const;
    bool is_date(const std::string& line) const;
    bool is_remark(const std::string& line) const;
    
    bool is_valid_event_line(const std::string& line, int line_number, std::set<Error>& errors) const;

private:
    const ConverterConfig& config_;
    std::unordered_set<std::string> valid_event_keywords_;
    std::unordered_set<std::string> wake_keywords_;
};

} // namespace txt
} // namespace validator

#endif // VALIDATOR_TXT_RULES_LINE_RULES_HPP_