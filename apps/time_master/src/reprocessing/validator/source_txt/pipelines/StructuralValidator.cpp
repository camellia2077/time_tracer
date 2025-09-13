#include "StructuralValidator.hpp"

void StructuralValidator::process_year_line(int line_number, const std::string& line, std::set<Error>& errors) {
    has_seen_year_ = true;
    has_seen_date_in_block_ = false;
    has_seen_event_in_day_ = false;
}

void StructuralValidator::process_date_line(int line_number, const std::string& line, std::set<Error>& errors) {
    if (!has_seen_year_) {
        // This case is handled by the main validator loop, but we can double check.
        errors.insert({line_number, "A date line must be preceded by a year header.", ErrorType::Source_NoDateAtStart});
    }
    has_seen_date_in_block_ = true;
    has_seen_event_in_day_ = false;
}

void StructuralValidator::process_remark_line(int line_number, const std::string& line, std::set<Error>& errors) {
    if (!has_seen_date_in_block_) {
        errors.insert({line_number, "A remark must follow a date line.", ErrorType::Source_InvalidLineFormat});
    }
    if (has_seen_event_in_day_) {
        errors.insert({line_number, "Remark lines cannot appear after an event line for the same day.", ErrorType::Source_RemarkAfterEvent});
    }
}

void StructuralValidator::process_event_line(int line_number, const std::string& line, std::set<Error>& errors) {
    if (!has_seen_date_in_block_) {
        errors.insert({line_number, "An event must follow a date line.", ErrorType::Source_InvalidLineFormat});
    }
    has_seen_event_in_day_ = true;
}

void StructuralValidator::process_unrecognized_line(int line_number, const std::string& line, std::set<Error>& errors) {
    if (!has_seen_year_) {
        errors.insert({line_number, "The file must start with a year header (e.g., 'y2025').", ErrorType::Source_MissingYearHeader});
    } else if (!has_seen_date_in_block_) {
        errors.insert({line_number, "A 4-digit date (MMDD) must follow the year header.", ErrorType::Source_NoDateAtStart});
    } else {
        errors.insert({line_number, "Invalid format. Must be a year (yYYYY), date (MMDD), remark, or a valid event.", ErrorType::Source_InvalidLineFormat});
    }
}

bool StructuralValidator::has_seen_year() const {
    return has_seen_year_;
}