#include "LineProcessor.hpp"
#include <algorithm>

LineProcessor::LineProcessor(const SourceValidatorConfig& config) : config_(config) {}

bool LineProcessor::is_year(const std::string& line) const {
    if (line.length() != 5 || line[0] != 'y') {
        return false;
    }
    return std::all_of(line.begin() + 1, line.end(), ::isdigit);
}

bool LineProcessor::is_date(const std::string& line) const {
    return line.length() == 4 && std::all_of(line.begin(), line.end(), ::isdigit);
}

bool LineProcessor::is_remark(const std::string& line) const {
    const std::string& prefix = config_.get_remark_prefix();
    if (prefix.empty() || line.rfind(prefix, 0) != 0) return false;
    return !trim(line.substr(prefix.length())).empty();
}

bool LineProcessor::is_valid_event_line(const std::string& line, int line_number, std::set<Error>& errors) const {
    if (line.length() < 5 || !std::all_of(line.begin(), line.begin() + 4, ::isdigit)) {
        return false;
    }
    try {
        int hh = std::stoi(line.substr(0, 2));
        int mm = std::stoi(line.substr(2, 2));
        if (hh > 23 || mm > 59) return false;
        
        std::string remaining_line = line.substr(4);
        std::string description;
        
        size_t comment_pos = std::string::npos;
        const char* delimiters[] = {"//", "#", ";"};
        for (const char* delim : delimiters) {
            size_t pos = remaining_line.find(delim);
            if (pos != std::string::npos && (comment_pos == std::string::npos || pos < comment_pos)) {
                comment_pos = pos;
            }
        }
        
        description = trim(remaining_line.substr(0, comment_pos));
        if (description.empty()) return false;

        const auto& wake_keywords = config_.get_wake_keywords();
        const auto& valid_keywords = config_.get_valid_event_keywords();
        if (wake_keywords.count(description) == 0 && valid_keywords.count(description) == 0) {
             errors.insert({line_number, "Unrecognized activity '" + description + "'. Please check spelling or update config file.", ErrorType::UnrecognizedActivity});
        }
        return true; 
    } catch (const std::exception&) {
        return false;
    }
}