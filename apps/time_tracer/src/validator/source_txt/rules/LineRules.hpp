// validator/source_txt/rules/LineRules.hpp
#ifndef LINE_RULES_HPP
#define LINE_RULES_HPP

#include "converter/config/ConverterConfig.hpp"
#include "validator/common/ValidatorUtils.hpp"
#include <string>
#include <set>
#include <unordered_set>

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

#endif // LINE_RULES_HPP