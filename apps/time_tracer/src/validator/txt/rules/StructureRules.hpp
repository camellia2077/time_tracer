// validator/txt/rules/StructureRules.hpp
#ifndef VALIDATOR_TXT_RULES_STRUCTURE_RULES_HPP_
#define VALIDATOR_TXT_RULES_STRUCTURE_RULES_HPP_

#include "validator/common/ValidatorUtils.hpp"
#include <string>
#include <set>

namespace validator {
namespace txt {

class StructureRules {
public:
    StructureRules() = default;

    void process_year_line(int line_number, const std::string& line, std::set<Error>& errors);
    void process_date_line(int line_number, const std::string& line, std::set<Error>& errors);
    void process_remark_line(int line_number, const std::string& line, std::set<Error>& errors);
    void process_event_line(int line_number, const std::string& line, std::set<Error>& errors);
    void process_unrecognized_line(int line_number, const std::string& line, std::set<Error>& errors);
    
    bool has_seen_year() const;

private:
    bool has_seen_year_ = false;
    bool has_seen_date_in_block_ = false;
    bool has_seen_event_in_day_ = false;
    int last_seen_year_ = 0; 
};

} // namespace txt
} // namespace validator

#endif // VALIDATOR_TXT_RULES_STRUCTURE_RULES_HPP_