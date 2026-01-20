// validator/txt/rules/StructureRules.cpp
#include "StructureRules.hpp"

namespace validator {
namespace txt {

void StructureRules::reset() {
    has_seen_year_ = false;
    has_seen_date_in_block_ = false;
    has_seen_event_in_day_ = false;
    has_seen_any_date_ = false;
    last_seen_year_ = 0;
}

void StructureRules::process_year_line(int line_number, const std::string& line, std::set<Error>& errors) {
    // [规则 1] 单文件只允许一个年份头
    if (has_seen_year_) {
        errors.insert({line_number, "Multiple year headers found. Only one year header is allowed per file (single month/year per file).", ErrorType::Structural});
        return;
    }

    int current_year = 0;
    try {
        current_year = std::stoi(line.substr(1));
    } catch (const std::exception&) {
        errors.insert({line_number, "Invalid year format.", ErrorType::Structural});
        return;
    }
    
    has_seen_year_ = true;
    last_seen_year_ = current_year;
    has_seen_date_in_block_ = false;
}

void StructureRules::process_date_line(int line_number, const std::string& line, std::set<Error>& errors) {
    if (!has_seen_year_) {
        errors.insert({line_number, "Date found before a year header.", ErrorType::Structural});
    }

    // [规则 2] 文件的第一个日期必须是该月的第一天 (MM01)
    if (!has_seen_any_date_) {
        // line 格式已被 LineRules 验证为 4位数字 (MMDD)
        if (line.length() >= 4) {
            std::string day_part = line.substr(2, 2);
            if (day_part != "01") {
                errors.insert({line_number, "The first date in the file must be the 1st day of the month (e.g., 0101). Found: " + line, ErrorType::DateContinuity});
            }
        }
        has_seen_any_date_ = true;
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