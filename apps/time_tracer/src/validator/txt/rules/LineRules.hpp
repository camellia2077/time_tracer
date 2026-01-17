// validator/txt/rules/LineRules.hpp
#ifndef VALIDATOR_TXT_RULES_LINE_RULES_HPP_
#define VALIDATOR_TXT_RULES_LINE_RULES_HPP_

#include "converter/config/ConverterConfig.hpp"
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
    
    // 注意：Error 在 validator 命名空间中，这里可以直接使用，或者需要 validator::Error
    // 因为 txt 是 validator 的子空间，所以可以直接访问父空间的符号
    bool is_valid_event_line(const std::string& line, int line_number, std::set<Error>& errors) const;

private:
    const ConverterConfig& config_;
    std::unordered_set<std::string> valid_event_keywords_;
    std::unordered_set<std::string> wake_keywords_;
};

} // namespace txt
} // namespace validator

#endif // VALIDATOR_TXT_RULES_LINE_RULES_HPP_