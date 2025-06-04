#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <unordered_set> // 新增：为了 std::unordered_set
#include <sstream>
#include <regex>
#include <algorithm>
#include <chrono>
#include <iomanip> // For std::put_time, std::get_time if used, and setw, setfill for time parsing

#include "json.hpp" //  This line requires the nlohmann/json.hpp file

// For directory traversal (C++17)
#include <filesystem>
namespace fs = std::filesystem;

// ANSI Color Codes
const std::string RESET_COLOR = "\033[0m";
const std::string RED_COLOR = "\033[31m";
const std::string GREEN_COLOR = "\033[32m";
const std::string YELLOW_COLOR = "\033[33m";

struct Config {
    std::map<std::string, std::unordered_set<std::string>> parent_categories; // 从 vector 改为 unordered_set
    bool loaded = false;
};

// Error structure
struct Error {
    int line_number;
    std::string message;

    bool operator<(const Error& other) const {
        if (line_number != other.line_number) {
            return line_number < other.line_number;
        }
        return message < other.message; // Secondary sort by message for uniqueness
    }
};

// Structure to hold data for a single date block during parsing
struct DateBlock {
    int start_line_number = -1;
    std::string date_str;
    int date_line_num = -1;

    std::string status_str_from_file; // e.g., "Status:True"
    bool status_value_from_file = false; // true if "Status:True"
    bool status_format_valid = false;
    int status_line_num = -1;

    std::string getup_str;
    int getup_line_num = -1;

    std::string remark_str;
    int remark_line_num = -1;

    std::vector<std::pair<std::string, int>> activity_lines_content; // pair of <activity_text, line_num>

    bool header_completely_valid = true; // Becomes false if Date, Status, Getup, or Remark has issues

    void reset() {
        start_line_number = -1;
        date_str.clear();
        date_line_num = -1;
        status_str_from_file.clear();
        status_value_from_file = false;
        status_format_valid = false;
        status_line_num = -1;
        getup_str.clear();
        getup_line_num = -1;
        remark_str.clear();
        remark_line_num = -1;
        activity_lines_content.clear();
        header_completely_valid = true;
    }
};

// --- Utility Functions ---
std::string trim(const std::string& str) {
    const std::string whitespace = " \t\n\r\f\v";
    size_t start = str.find_first_not_of(whitespace);
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(whitespace);
    return str.substr(start, end - start + 1);
}

bool parse_time_format(const std::string& time_str, int& hours, int& minutes) {
    if (time_str.length() != 5 || time_str[2] != ':') return false;
    try {
        hours = std::stoi(time_str.substr(0, 2));
        minutes = std::stoi(time_str.substr(3, 2));
        if (hours < 0 || hours > 23 || minutes < 0 || minutes > 59) return false;
        if (time_str.substr(0, 2).length() != 2 || time_str.substr(3, 2).length() != 2) return false;
        if (!isdigit(time_str[0]) || !isdigit(time_str[1]) || !isdigit(time_str[3]) || !isdigit(time_str[4])) return false;
    } catch (const std::exception&) {
        return false;
    }
    return true;
}
Config load_configuration(const std::string& config_file_path) {
    Config config;
    std::ifstream ifs(config_file_path);
    if (!ifs.is_open()) {
        std::cerr << YELLOW_COLOR << "Warning: Configuration file \"" << config_file_path << "\" not found or could not be opened." << RESET_COLOR << std::endl;
        return config;
    }

    try {
        nlohmann::json j;
        ifs >> j;
        if (j.contains("PARENT_CATEGORIES") && j["PARENT_CATEGORIES"].is_object()) {
            for (auto const& [parent_key, val] : j["PARENT_CATEGORIES"].items()) {
                if (val.is_array()) {
                    std::unordered_set<std::string> sub_tags_set; // 使用 unordered_set
                    for (const auto& item : val) {
                        if (item.is_string()) {
                            sub_tags_set.insert(item.get<std::string>()); // 插入到 unordered_set
                        } else {
                            throw std::runtime_error("Sub-tag in PARENT_CATEGORIES is not a string for parent: " + parent_key);
                        }
                    }
                    config.parent_categories[parent_key] = sub_tags_set;
                } else {
                     throw std::runtime_error("Value for parent category \"" + parent_key + "\" is not an array.");
                }
            }
            config.loaded = true;
        } else {
            throw std::runtime_error("PARENT_CATEGORIES key missing or not an object in JSON.");
        }
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << YELLOW_COLOR << "Warning: JSON parsing error in \"" << config_file_path << "\": " << e.what() << RESET_COLOR << std::endl;
    } catch (const std::exception& e) {
        std::cerr << YELLOW_COLOR << "Warning: Error processing configuration file \"" << config_file_path << "\": " << e.what() << RESET_COLOR << std::endl;
    }
    
    if (!config.loaded) {
         std::cerr << YELLOW_COLOR << "Continuing without category rules or with an empty configuration." << RESET_COLOR << std::endl;
    }
    return config;
}


// --- Validation Logic --- // 修改点：regex 对象改为 static const
void validate_date_line(const std::string& line, int line_num, DateBlock& current_block, std::set<Error>& errors) {
    current_block.date_line_num = line_num;
    static const std::regex date_regex("^Date:(\\d{8})$"); // 改为 static const
    std::smatch match;
    if (std::regex_match(line, match, date_regex)) {
        current_block.date_str = match[1].str();
    } else {
        errors.insert({line_num, "Date line format error. Expected \"Date:YYYYMMDD\"."});
        current_block.header_completely_valid = false;
    }
}

void validate_status_line(const std::string& line, int line_num, DateBlock& current_block, std::set<Error>& errors) {
    current_block.status_line_num = line_num;
    current_block.status_str_from_file = line;
    if (line == "Status:True") {
        current_block.status_value_from_file = true;
        current_block.status_format_valid = true;
    } else if (line == "Status:False") {
        current_block.status_value_from_file = false;
        current_block.status_format_valid = true;
    } else {
        errors.insert({line_num, "Status line format error. Expected \"Status:True\" or \"Status:False\"."});
        current_block.status_format_valid = false;
        current_block.header_completely_valid = false;
    }
}

void validate_getup_line(const std::string& line, int line_num, DateBlock& current_block, std::set<Error>& errors) {
    current_block.getup_line_num = line_num;
    static const std::regex getup_regex("^Getup:(\\d{2}:\\d{2})$");
    std::smatch match;
    if (std::regex_match(line, match, getup_regex)) {
        current_block.getup_str = match[1].str();
        int hh, mm;
        if (!parse_time_format(current_block.getup_str, hh, mm)) {
            errors.insert({line_num, "Getup time value invalid. Ensure HH is 00-23 and MM is 00-59, both two digits."});
            current_block.header_completely_valid = false;
        }
    } else {
        errors.insert({line_num, "Getup line format error. Expected \"Getup:HH:MM\"."});
        current_block.header_completely_valid = false;
    }
}

void validate_remark_line(const std::string& line, int line_num, DateBlock& current_block, std::set<Error>& errors) {
    current_block.remark_line_num = line_num;
    if (line.rfind("Remark:", 0) != 0) { 
        errors.insert({line_num, "Remark line format error. Expected \"Remark:\" prefix."});
        current_block.header_completely_valid = false;
    }
    current_block.remark_str = line;
}

// MODIFIED FUNCTION
void validate_activity_line(const std::string& line, int line_num, const Config& config, DateBlock& current_block, std::set<Error>& errors) {
    static const std::regex activity_regex("^(\\d{2}:\\d{2})~(\\d{2}:\\d{2})(.+)$");
    std::smatch match;

    if (std::regex_match(line, match, activity_regex)) {
        std::string start_time_str = match[1].str();
        std::string end_time_str = match[2].str();
        std::string activity_text = trim(match[3].str());

        int start_hh, start_mm, end_hh, end_mm;
        bool start_valid = parse_time_format(start_time_str, start_hh, start_mm);
        bool end_valid = parse_time_format(end_time_str, end_hh, end_mm);

        if (!start_valid) {
            errors.insert({line_num, "Activity start time invalid format or value."});
        }
        if (!end_valid) {
            errors.insert({line_num, "Activity end time invalid format or value."});
        }

        if (start_valid && end_valid) {
            if (start_hh > end_hh) {
                // This case is allowed for overnight activities, no error here.
                // However, the problem description implies activities are within the same day.
                // For now, let's assume end time must be after start time if on the same day.
                // If strict same-day: errors.insert({line_num, "Activity end time cannot be earlier than start time."});
            } else if (start_hh == end_hh && end_mm < start_mm) {
                 errors.insert({line_num, "Activity end time minutes cannot be less than start time minutes if hours are the same."});
            }
        }

        // --- Start of new/modified activity text validation ---
        bool text_format_rules_passed = true; // Flag for all text format rules

        static const std::regex text_content_regex("^[a-zA-Z0-9_-]+$");
        if (!std::regex_match(activity_text, text_content_regex)) {
            errors.insert({line_num, "Activity text content \"" + activity_text + "\" contains invalid characters. Only letters, numbers, underscore (_), hyphen (-) allowed."});
            text_format_rules_passed = false;
        }

        // New Rule 1: Must contain an underscore
        if (text_format_rules_passed) { // Only proceed if basic regex passed
            if (activity_text.find('_') == std::string::npos) {
                errors.insert({line_num, "Activity text \"" + activity_text + "\" must contain an underscore ('_')."});
                text_format_rules_passed = false;
            }
        }

        // New Rule 2: Cannot end with an underscore
        if (text_format_rules_passed) { // Only proceed if previous rules also passed
            // activity_text is guaranteed non-empty here due to previous checks if text_format_rules_passed is true
            if (activity_text.back() == '_') {
                errors.insert({line_num, "Activity text \"" + activity_text + "\" cannot end with an underscore ('_')."});
                text_format_rules_passed = false;
            }
        }
        // --- End of new/modified activity text validation ---


        if (text_format_rules_passed) { // Proceed only if all text format rules (regex, underscore presence, no trailing underscore) passed
            if (config.loaded && !config.parent_categories.empty()) {
                bool tag_is_valid_by_config = false;
                bool tag_matched_a_parent_prefix_rule = false;

                if (config.parent_categories.count(activity_text)) {
                    errors.insert({line_num, "Activity text \"" + activity_text + "\" cannot be a parent category name."});
                } else {
                    for (const auto& pair : config.parent_categories) {
                        const std::string& parent_name = pair.first;
                        const std::unordered_set<std::string>& allowed_sub_tags_set = pair.second;
                        std::string prefix = parent_name + "_";

                        if (activity_text.rfind(prefix, 0) == 0) {
                            tag_matched_a_parent_prefix_rule = true;
                            if (allowed_sub_tags_set.count(activity_text)) {
                                tag_is_valid_by_config = true;
                            } else {
                                std::string examples_str;
                                if (!allowed_sub_tags_set.empty()) {
                                    std::vector<std::string> sorted_examples(allowed_sub_tags_set.begin(), allowed_sub_tags_set.end());
                                    std::sort(sorted_examples.begin(), sorted_examples.end());
                                    examples_str = "Allowed in \"" + parent_name + "\" include: ";
                                    for (size_t i = 0; i < std::min(sorted_examples.size(), static_cast<size_t>(3)); ++i) {
                                        examples_str += "\"" + sorted_examples[i] + "\"" + (i < std::min(sorted_examples.size(), static_cast<size_t>(3)) - 1 ? ", " : "");
                                    }
                                    if (sorted_examples.size() > 3) examples_str += ", etc.";
                                }
                                errors.insert({line_num, "Activity text \"" + activity_text + "\" is not an allowed sub-tag for parent '" + parent_name + "'." + examples_str});
                            }
                            break;
                        }
                    }
                    if (!tag_matched_a_parent_prefix_rule && !tag_is_valid_by_config) {
                        std::string all_examples_str;
                        std::vector<std::string> all_possible_tags;
                        for(const auto& p_cat_pair : config.parent_categories) {
                            for(const auto& tag : p_cat_pair.second) {
                                all_possible_tags.push_back(tag);
                            }
                        }
                        if(!all_possible_tags.empty()){
                            std::sort(all_possible_tags.begin(), all_possible_tags.end());
                            all_examples_str = " Some allowed activities include: ";
                             for (size_t i = 0; i < std::min(all_possible_tags.size(), static_cast<size_t>(3)); ++i) {
                                all_examples_str += "\"" + all_possible_tags[i] + "\"" + (i < std::min(all_possible_tags.size(), static_cast<size_t>(3)) - 1 ? ", " : "");
                            }
                            if (all_possible_tags.size() > 3) all_examples_str += ", etc.";
                        }

                        errors.insert({line_num, "Activity text \"" + activity_text + "\" does not conform to any defined parent category prefix structures or is not a recognized activity." + all_examples_str});
                    }
                }
                 if(tag_is_valid_by_config) {
                    current_block.activity_lines_content.push_back({activity_text, line_num});
                 }
            } else { // Config not loaded or empty, and text_format_rules_passed is true
                 current_block.activity_lines_content.push_back({activity_text, line_num});
            }
        }
    } else {
        errors.insert({line_num, "Activity line format error. Expected \"HH:MM~HH:MMTextContent\"."});
    }
}


void finalize_block_status_validation(DateBlock& block, std::set<Error>& errors) {
    if (!block.status_format_valid) {
        return;
    }
    if (block.status_line_num == -1) return;

    bool contains_study = false;
    for (const auto& activity_pair : block.activity_lines_content) {
        // Modified to check for "study" as a whole word or part of a valid underscore-separated tag.
        // Example: "category_study", "study_project"
        // This simple find should still work if "study" is required to be part of an activity name.
        if (activity_pair.first.find("study") != std::string::npos) {
            contains_study = true;
            break;
        }
    }

    if (contains_study && !block.status_value_from_file) {
        errors.insert({block.status_line_num, "Status: line must be \"Status:True\" because \"study\" was found in activities for date " + block.date_str + "."});
    } else if (!contains_study && block.status_value_from_file) {
        errors.insert({block.status_line_num, "Status: line must be \"Status:False\" because \"study\" was not found in any activities for date " + block.date_str + "."});
    }
}

// --- File Processing ---
bool process_file(const std::string& file_path, const Config& config, std::set<Error>& file_errors_out) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        std::cerr << RED_COLOR << "Error: Could not open file: " << file_path << RESET_COLOR << std::endl;
        return true; 
    }

    std::string line;
    int line_number = 0;
    
    DateBlock current_block;
    bool first_non_empty_line_in_file = true;
    bool date_line_encountered_for_file = false;
    
    enum class ParseState { EXPECT_DATE, EXPECT_STATUS, EXPECT_GETUP, EXPECT_REMARK, EXPECT_ACTIVITY };
    ParseState current_parse_state = ParseState::EXPECT_DATE;

    auto finalize_previous_block = [&](DateBlock& block_to_finalize) {
        if (block_to_finalize.date_line_num != -1) { 
            if (block_to_finalize.header_completely_valid) { 
                 finalize_block_status_validation(block_to_finalize, file_errors_out);
            }
            block_to_finalize.reset(); 
        }
    };

    while (std::getline(file, line)) {
        line_number++;
        std::string trimmed_line = trim(line);

        if (first_non_empty_line_in_file && !trimmed_line.empty()) {
            first_non_empty_line_in_file = false;
            if (trimmed_line.rfind("Date:", 0) != 0) {
                file_errors_out.insert({line_number, "File (if non-empty) must begin with a valid Date: line."});
                current_block.header_completely_valid = false; 
            }
        }
        
        if (trimmed_line.empty()) {
            continue;
        }

        if (trimmed_line.rfind("Date:", 0) == 0) {
            if (date_line_encountered_for_file) { 
                if (current_parse_state != ParseState::EXPECT_ACTIVITY && current_parse_state != ParseState::EXPECT_DATE) {
                     file_errors_out.insert({current_block.start_line_number != -1 ? current_block.start_line_number : line_number,
                                           "New Date: line found before previous block was complete (expected S/G/R or activity)."});
                }
                 finalize_previous_block(current_block);
            } else { 
                 current_block.reset(); 
            }
            
            date_line_encountered_for_file = true;
            current_block.start_line_number = line_number;
            validate_date_line(trimmed_line, line_number, current_block, file_errors_out);
            if (current_block.header_completely_valid) {
                current_parse_state = ParseState::EXPECT_STATUS;
            } else {
                current_parse_state = ParseState::EXPECT_DATE; 
            }
        } else { 
            if (!date_line_encountered_for_file) {
                if (!first_non_empty_line_in_file) { 
                    file_errors_out.insert({line_number, "Expected Date: line to start a block."});
                }
                current_parse_state = ParseState::EXPECT_DATE; 
                continue; 
            }

            if (!current_block.header_completely_valid && current_parse_state != ParseState::EXPECT_DATE) {
                // If header (Date, Status, Getup, Remark) had an error, 
                // we might not want to process activities for this block,
                // or we might still want to report activity errors.
                // Current logic: if header invalid, subsequent lines might be skipped or misparsed by state machine.
                // For now, if a header part fails, it might prevent further parsing of that block's items correctly.
                // This `continue` means if e.g. Date was bad, we won't try to parse Status, Getup, etc. or activities.
                // This might be okay, as the block is already marked problematic.
                continue;
            }

            switch (current_parse_state) {
                case ParseState::EXPECT_STATUS:
                    validate_status_line(trimmed_line, line_number, current_block, file_errors_out);
                    // Only proceed if Status itself was okay, or always proceed?
                    // The `header_completely_valid` flag on current_block handles whether date/status/getup/remark had errors.
                    current_parse_state = ParseState::EXPECT_GETUP;
                    break;
                case ParseState::EXPECT_GETUP:
                    validate_getup_line(trimmed_line, line_number, current_block, file_errors_out);
                    current_parse_state = ParseState::EXPECT_REMARK;
                    break;
                case ParseState::EXPECT_REMARK:
                    validate_remark_line(trimmed_line, line_number, current_block, file_errors_out);
                    current_parse_state = ParseState::EXPECT_ACTIVITY;
                    break;
                case ParseState::EXPECT_ACTIVITY:
                    validate_activity_line(trimmed_line, line_number, config, current_block, file_errors_out);
                    break;
                case ParseState::EXPECT_DATE: 
                    // This state means we are waiting for a new Date line, likely due to an error in the previous one
                    // or it's the very start of the file processing after an initial non-Date line.
                    // Non-Date lines encountered here (if not empty) should ideally be flagged if they aren't a new Date line.
                    // However, the logic for `trimmed_line.rfind("Date:", 0) == 0` handles new Date lines.
                    // Other lines here are effectively ignored until a new Date starts or file ends.
                    break; 
            }
        }
    }

    if (date_line_encountered_for_file) { 
        if (current_parse_state != ParseState::EXPECT_ACTIVITY && current_parse_state != ParseState::EXPECT_DATE) {
             // This means the file ended after Date, or Date+Status, or Date+Status+Getup, or Date+Status+Getup+Remark, but before any activities or a new Date.
             // An error if we expect at least one activity, or if the block structure is strictly Date,S,G,R,Activity(s)
             file_errors_out.insert({current_block.start_line_number != -1 ? current_block.start_line_number : line_number,
                                   "File ended before current block was complete (expected Status, Getup, Remark, or activity lines)."});
        }
       finalize_previous_block(current_block); // Finalize the last block
    } else if (!first_non_empty_line_in_file && line_number > 0) { // File had content but no Date: line
        if(file_errors_out.empty()){ // Add this error only if no other error (like "must begin with Date") was added for line 1
             file_errors_out.insert({1, "File (if non-empty) must begin with a valid Date: line. No Date: line found."});
        }
    } // If file is empty (line_number == 0), no errors.

    return !file_errors_out.empty();
}

int main(int argc, char* argv[]) {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL); 

    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <file.txt|directory_path>" << std::endl;
        return 1;
    }

    std::string config_path_env = "ACTIVITY_CONFIG_PATH";
    std::string config_file = "format_validator_config.json"; 
    const char* env_p = std::getenv(config_path_env.c_str());
    if (env_p != nullptr) {
        config_file = env_p;
        std::cout << "Using configuration file from " << config_path_env << ": " << config_file << std::endl;
    } else {
        std::cout << "Environment variable " << config_path_env << " not set. Using default '" << config_file << "' in current directory." << std::endl;
    }

    Config config = load_configuration(config_file); 
    if (!config.loaded) {
        std::cout << YELLOW_COLOR << "Running with potentially limited validation due to configuration issues." << RESET_COLOR << std::endl;
    } else {
        std::cout << GREEN_COLOR << "Configuration loaded successfully." << RESET_COLOR << std::endl;
    }

    std::string path_arg = argv[1];
    std::vector<std::string> files_to_process;
    bool is_directory = false;

    try {
        if (fs::is_directory(path_arg)) {
            is_directory = true;
            for (const auto& entry : fs::recursive_directory_iterator(path_arg)) {
                if (entry.is_regular_file() && entry.path().extension() == ".txt") {
                    files_to_process.push_back(entry.path().string());
                }
            }
        } else if (fs::is_regular_file(path_arg)) {
            if (fs::path(path_arg).extension() == ".txt") {
                files_to_process.push_back(path_arg);
            } else {
                std::cerr << RED_COLOR << "Error: Specified file is not a .txt file: " << path_arg << RESET_COLOR << std::endl;
                return 1;
            }
        } else {
            std::cerr << RED_COLOR << "Error: Path specified is not a valid file or directory: " << path_arg << RESET_COLOR << std::endl;
            return 1;
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << RED_COLOR << "Filesystem error: " << e.what() << RESET_COLOR << std::endl;
        return 1;
    }
    
    if (files_to_process.empty()) {
        std::cout << YELLOW_COLOR << "No .txt files found to process." << RESET_COLOR << std::endl;
        return 0;
    }

    int total_files_processed = 0;
    int files_with_errors = 0;
    std::chrono::duration<double, std::milli> accumulated_core_processing_time(0.0); 

    for (const auto& file_path : files_to_process) {
        std::cout << "Processing file: " << file_path << "..." << std::endl;
        std::set<Error> current_file_errors;

        auto individual_file_start_time = std::chrono::high_resolution_clock::now(); 
        bool has_errors = process_file(file_path, config, current_file_errors); 
        auto individual_file_end_time = std::chrono::high_resolution_clock::now(); 
        accumulated_core_processing_time += (individual_file_end_time - individual_file_start_time); 
        
        total_files_processed++;

        if (has_errors) {
            files_with_errors++;
            std::cout << RED_COLOR << "File: " << file_path << " - Errors found:" << RESET_COLOR << std::endl;
            for (const auto& err : current_file_errors) {
                std::cout << RED_COLOR << "  Line " << err.line_number << ": " << RESET_COLOR << err.message  << std::endl; 
            }
        } else {
            std::cout << GREEN_COLOR << "File: " << file_path << " - No errors found." << RESET_COLOR << std::endl;
        }
        std::cout << "----------------------------------------" << std::endl;
    }

    const int G_PRECISION = 6; 

    if (is_directory) {
        std::cout << "\n--- Directory Scan Summary ---" << std::endl;
        std::cout << "Total .txt files processed: " << total_files_processed << std::endl;
        std::cout << "Files with errors: " << (files_with_errors > 0 ? RED_COLOR : GREEN_COLOR) << files_with_errors << RESET_COLOR << std::endl;
        std::cout << "Total core processing time (sum of process_file calls): " 
                  << std::fixed << std::setprecision(G_PRECISION) 
                  << accumulated_core_processing_time.count() / 1000.0 << " seconds" << std::endl;
    } else if (total_files_processed == 1) { 
         std::cout << "\n--- File Processing Summary ---" << std::endl;
         if (files_with_errors > 0) {
             std::cout << RED_COLOR << "Errors found in " << path_arg << RESET_COLOR << std::endl;
         } else {
             std::cout << GREEN_COLOR << "No errors found in " << path_arg << RESET_COLOR << std::endl;
         }
         std::cout << "Core processing time (process_file call): " 
                   << std::fixed << std::setprecision(G_PRECISION) 
                   << accumulated_core_processing_time.count() / 1000.0 << " seconds" << std::endl;
    }

    return files_with_errors > 0 ? 1 : 0;
}
