// validator/source_txt/rules/StructureRules.hpp
#ifndef STRUCTURE_RULES_HPP
#define STRUCTURE_RULES_HPP

#include "validator/common/ValidatorUtils.hpp"
#include <string>
#include <set>

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

#endif // STRUCTURE_RULES_HPP