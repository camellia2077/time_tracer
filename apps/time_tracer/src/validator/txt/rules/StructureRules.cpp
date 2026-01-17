// validator/txt/rules/StructureRules.cpp
#include "StructureRules.hpp"

namespace validator {
namespace txt {

void StructureRules::process_year_line(int line_number, const std::string& line, std::set<Error>& errors) {
    int current_year = 0;
    try {
        current_year = std::stoi(line.substr(1));
    } catch (const std::exception&) {
        errors.insert({line_number, "Invalid year format.", ErrorType::Structural});
        return;
    }

    if (has_seen_year_) {
        if (current_year != last_seen_year_ && current_year != last_seen_year_ + 1) {
            errors.insert({line_number, "Non-sequential year header found. Years must be consecutive or identical (e.g., y2024 followed by y2025 or another y2024).", ErrorType::Structural});
        }
    }
    
    has_seen_year_ = true;
    last_seen_year_ = current_year;
    has_seen_date_in_block_ = false;
}

void StructureRules::process_date_line(int line_number, const std::string& /*line*/, std::set<Error>& errors) {
    if (!has_seen_year_) {
        errors.insert({line_number, "Date found before a year header.", ErrorType::Structural});
    }
    has_seen_date_in_block_ = true;
    has_seen_event_in_day_ = false;
}

void StructureRules::process_remark_line(int line_number, const std::string& /*line*/, std::set<Error>& errors) {
    if (!has_seen_date_in_block_) {
        errors.insert({line_number, "Remark found before a date.", ErrorType::Structural});
    }
    if (has_seen_event_in_day_) {
        errors.insert({line_number, "Remark must appear before any events for the day.", ErrorType::Source_RemarkAfterEvent});
    }
}

void StructureRules::process_event_line(int line_number, const std::string& /*line*/, std::set<Error>& errors) {
    if (!has_seen_date_in_block_) {
        errors.insert({line_number, "Event found before a date.", ErrorType::Structural});
    }
    has_seen_event_in_day_ = true;
}

void StructureRules::process_unrecognized_line(int line_number, const std::string& line, std::set<Error>& errors) {
    errors.insert({line_number, "Unrecognized line format: " + line, ErrorType::Source_InvalidLineFormat});
}

bool StructureRules::has_seen_year() const {
    return has_seen_year_;
}

} // namespace txt
} // namespace validator