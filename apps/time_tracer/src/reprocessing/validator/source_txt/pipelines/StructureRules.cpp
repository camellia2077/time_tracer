// reprocessing/validator/source_txt/pipelines/StructureRules.cpp
#include "StructureRules.hpp"

void StructureRules::process_year_line(int line_number, const std::string& line, std::set<Error>& errors) {
    // --- [核心修改] 重新设计年份验证逻辑 ---
    int current_year = 0;
    try {
        current_year = std::stoi(line.substr(1));
    } catch (const std::exception&) {
        errors.insert({line_number, "Invalid year format.", ErrorType::Structural});
        return;
    }

    if (has_seen_year_) {
        // 如果已经见过一个年份，只允许新的年份是上一个年份的延续
        if (current_year != last_seen_year_ + 1) {
            errors.insert({line_number, "Duplicate or non-sequential year header found. Only consecutive years are allowed (e.g., y2024 followed by y2025).", ErrorType::Structural});
        }
    }
    
    has_seen_year_ = true;
    last_seen_year_ = current_year;
    has_seen_date_in_block_ = false; // 为新年份块重置状态
}

void StructureRules::process_date_line(int line_number, const std::string& /*line*/, std::set<Error>& errors) {
    if (!has_seen_year_) {
        errors.insert({line_number, "Date found before a year header.", ErrorType::Structural});
    }
    has_seen_date_in_block_ = true;
    has_seen_event_in_day_ = false; // Reset for the new day
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