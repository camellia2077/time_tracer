#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <iomanip>
#include <optional>
#include <algorithm>
#include <chrono>
#include <filesystem> 
#include <sstream>

// Include the json.hpp header
// Make sure json.hpp is in your include path or in the same directory
#include "json.hpp" // Or <nlohmann/json.hpp> if installed differently

// Use nlohmann::json
using json = nlohmann::json;

// Helper function to trim leading/trailing whitespace
std::string trim_string(const std::string& str) {
    const std::string whitespace = " \t\n\r\f\v";
    size_t start = str.find_first_not_of(whitespace);
    if (start == std::string::npos)
        return ""; // string is empty or all whitespace
    size_t end = str.find_last_not_of(whitespace);
    return str.substr(start, end - start + 1);
}

// Helper function to split string by lines
std::vector<std::string> split_lines(const std::string& s) {
    std::vector<std::string> result;
    std::string line;
    std::istringstream ss(s);
    while (std::getline(ss, line)) {
        // Handle \r\n by removing \r if present at the end
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        result.push_back(line);
    }
    return result;
}

class LogProcessor {
public:
    static const std::string GREEN;
    static const std::string RED;
    static const std::string YELLOW;
    static const std::string RESET;

private:
    int year_to_use;
    std::optional<std::string> _last_interval_start_raw_time;
    bool _was_previous_event_initial_raw_xing;
    bool _printed_at_least_one_block;
    std::map<std::string, std::string> TEXT_REPLACEMENT_MAP;
    std::ostream& output_stream_ref; // Reference to the output stream
    bool is_writing_to_file;          // Flag to indicate if output is to a file (for color stripping)

public:
    LogProcessor(int year = 2025, const std::string& replacement_map_file = "end2duration.json", std::ostream& out_stream = std::cout, bool is_file = false)
        : year_to_use(year),
          _last_interval_start_raw_time(std::nullopt),
          _was_previous_event_initial_raw_xing(false),
          _printed_at_least_one_block(false),
          output_stream_ref(out_stream),
          is_writing_to_file(is_file) {
        TEXT_REPLACEMENT_MAP = _load_replacement_map(replacement_map_file);
    }

    void _output(const std::string& message) {
        if (is_writing_to_file) {
            std::string plain_message = message;
            std::string codes_to_strip[] = {GREEN, RED, YELLOW, RESET};
            for (const auto& code : codes_to_strip) {
                size_t pos = plain_message.find(code);
                while (pos != std::string::npos) {
                    plain_message.replace(pos, code.length(), "");
                    pos = plain_message.find(code, pos); // Start search from current pos to avoid issues if replacement is part of another code
                }
            }
            output_stream_ref << plain_message << std::endl;
        } else {
            output_stream_ref << message << std::endl;
        }
    }

private:
    std::map<std::string, std::string> _load_replacement_map(const std::string& file_path) {
        if (!std::filesystem::exists(file_path)) {
            std::cerr << "警告: 文本替换映射文件 '" << file_path << "' 未找到。将使用空的替换映射。" << std::endl;
            return {};
        }
        try {
            std::ifstream f(file_path);
            json data = json::parse(f);
            if (!data.is_object()) {
                std::cerr << "警告: 文本替换映射文件 '" << file_path << "' 的内容不是一个有效的JSON对象 (字典)。将使用空的替换映射。" << std::endl;
                return {};
            }
            return data.get<std::map<std::string, std::string>>();
        } catch (json::parse_error& e) {
            std::cerr << "警告: 解析文本替换映射文件 '" << file_path << "' 失败。请确保它是有效的JSON格式。错误: " << e.what() << "。将使用空的替换映射。" << std::endl;
            return {};
        } catch (const std::exception& e) {
            std::cerr << "读取文本替换映射文件 '" << file_path << "' 时发生未知错误: " << e.what() << "。将使用空的替换映射。" << std::endl;
            return {};
        }
    }

    static std::string _format_time(const std::string& time_str) {
        if (time_str.length() == 4 && std::all_of(time_str.begin(), time_str.end(), ::isdigit)) {
            return time_str.substr(0, 2) + ":" + time_str.substr(2, 2);
        }
        return time_str;
    }

    void _reset_date_block_processing_state() {
        _last_interval_start_raw_time = std::nullopt;
        _was_previous_event_initial_raw_xing = false;
    }

    void _process_and_print_date_block(const std::string& date_line_content, const std::vector<std::string>& event_lines_content) {
        std::string month_chars, day_chars;
        int month_int = 0, day_int = 0;

        if (date_line_content.length() == 4 && std::all_of(date_line_content.begin(), date_line_content.end(), ::isdigit)) {
            month_chars = date_line_content.substr(0, 2);
            day_chars = date_line_content.substr(2, 2);
            try {
                month_int = std::stoi(month_chars);
                day_int = std::stoi(day_chars);
                if (!(month_int >= 1 && month_int <= 12 && day_int >= 1 && day_int <= 31)) {
                    std::cerr << "警告: 无效的日期部分 '" << date_line_content << "'. 跳过此日期块。" << std::endl;
                    return;
                }
            } catch (const std::invalid_argument& ia) {
                std::cerr << "警告: 无法将日期部分 '" << date_line_content << "' 解析为数字。跳过此日期块。" << std::endl;
                return;
            } catch (const std::out_of_range& oor) {
                 std::cerr << "警告: 日期部分 '" << date_line_content << "' 超出范围。跳过此日期块。" << std::endl;
                return;
            }
        } else {
            std::cerr << "警告: 日期行格式无效 '" << date_line_content << "'. 跳过此日期块。" << std::endl;
            return;
        }

        std::ostringstream oss_date;
        oss_date << "Date:" << year_to_use
                 << std::setw(2) << std::setfill('0') << month_int
                 << std::setw(2) << std::setfill('0') << day_int;
        std::string formatted_date_output_str = oss_date.str();

        bool day_has_study_event = false;
        std::vector<std::string> event_related_output_lines;
        _reset_date_block_processing_state();
        bool is_first_event_actual_getup_type = false;
        std::string first_event_true_formatted_time;

        if (!event_lines_content.empty()) {
            const std::string& first_event_line_peek_str = event_lines_content[0];
            if (first_event_line_peek_str.length() >= 4 && std::all_of(first_event_line_peek_str.substr(0, 4).begin(), first_event_line_peek_str.substr(0, 4).end(), ::isdigit)) {
                std::string first_event_peek_raw_time = first_event_line_peek_str.substr(0, 4);
                std::string first_event_peek_original_text = first_event_line_peek_str.substr(4);
                if (first_event_peek_original_text == "醒" || first_event_peek_original_text == "起床") {
                    is_first_event_actual_getup_type = true;
                    first_event_true_formatted_time = _format_time(first_event_peek_raw_time);
                }
            }
        }

        if (is_first_event_actual_getup_type) {
            event_related_output_lines.push_back("Getup:" + first_event_true_formatted_time);
            event_related_output_lines.push_back("Remark:");
        } else {
            event_related_output_lines.push_back("Getup:" + YELLOW + "null" + RESET);
            event_related_output_lines.push_back("Remark:");
        }

        size_t start_processing_from_index = 0;
        if (is_first_event_actual_getup_type && !event_lines_content.empty()) {
            std::string consumed_event_raw_time = event_lines_content[0].substr(0, 4);
            std::string consumed_event_original_text = event_lines_content[0].substr(4);
            _last_interval_start_raw_time = consumed_event_raw_time;
            _was_previous_event_initial_raw_xing = (consumed_event_original_text == "醒");
            
            std::string consumed_display_text = consumed_event_original_text;
            auto it = TEXT_REPLACEMENT_MAP.find(consumed_event_original_text);
            if (it != TEXT_REPLACEMENT_MAP.end()) {
                consumed_display_text = it->second;
            }
            if (consumed_display_text.find("study") != std::string::npos) {
                day_has_study_event = true;
            }
            start_processing_from_index = 1;
        }

        for (size_t i = start_processing_from_index; i < event_lines_content.size(); ++i) {
            const std::string& event_line_str = event_lines_content[i];
            std::string raw_time = event_line_str.substr(0, 4);
            std::string original_text = event_line_str.substr(4);
            std::string current_formatted_time = _format_time(raw_time);
            
            std::string display_text = original_text;
            auto it = TEXT_REPLACEMENT_MAP.find(original_text);
            if (it != TEXT_REPLACEMENT_MAP.end()) {
                display_text = it->second;
            }

            if (display_text.find("study") != std::string::npos) {
                day_has_study_event = true;
            }

            if (!_last_interval_start_raw_time.has_value()) {
                event_related_output_lines.push_back(current_formatted_time + display_text);
                _last_interval_start_raw_time = raw_time;
                _was_previous_event_initial_raw_xing = false; 
            } else {
                std::string start_formatted_time = _format_time(_last_interval_start_raw_time.value());
                event_related_output_lines.push_back(start_formatted_time + "~" + current_formatted_time + display_text);
                _was_previous_event_initial_raw_xing = false;
                _last_interval_start_raw_time = raw_time;
            }
        }

        _output(formatted_date_output_str);
        if (day_has_study_event) {
            _output(GREEN + "Status:True" + RESET);
        } else {
            _output(RED + "Status:False" + RESET);
        }

        for (const auto& out_line : event_related_output_lines) {
            _output(out_line);
        }
    }

public:
    void process_log_data(const std::string& log_data_str) {
        std::vector<std::string> lines = split_lines(log_data_str);
        std::optional<std::string> current_date_raw_content = std::nullopt;
        std::vector<std::string> current_event_lines_for_block;
        _printed_at_least_one_block = false;

        for (const auto& line_content_untrimmed : lines) {
            std::string line = trim_string(line_content_untrimmed);
            if (line.empty()) {
                continue;
            }

            bool is_date_line = (line.length() == 4 && std::all_of(line.begin(), line.end(), ::isdigit));

            if (is_date_line) {
                if (current_date_raw_content.has_value()) {
                    if (_printed_at_least_one_block) {
                        _output(""); // Print empty line between blocks
                    }
                    _process_and_print_date_block(current_date_raw_content.value(), current_event_lines_for_block);
                    _printed_at_least_one_block = true;
                }
                current_date_raw_content = line;
                current_event_lines_for_block.clear();
            } else { // Assumed event line
                bool is_valid_event_format = (line.length() >= 4 && line.substr(0, 4).length() == 4 &&
                                             std::all_of(line.substr(0, 4).begin(), line.substr(0, 4).end(), ::isdigit));
                if (is_valid_event_format) {
                    if (current_date_raw_content.has_value()) {
                        current_event_lines_for_block.push_back(line);
                    } else {
                        std::cerr << "警告: 发现事件行 '" << line << "' 但没有活动的日期块。此行将被忽略。" << std::endl;
                    }
                } else {
                    // Optionally warn about malformed lines not matching date or event structure
                    // std::cerr << "警告: 忽略格式不符的行: '" << line << "'" << std::endl;
                }
            }
        }

        // Process the last date block if it exists
        if (current_date_raw_content.has_value()) {
            if (_printed_at_least_one_block && !current_event_lines_for_block.empty()) {
                 _output("");
            }
            _process_and_print_date_block(current_date_raw_content.value(), current_event_lines_for_block);
        }
    }
};

// Initialize static const members
const std::string LogProcessor::GREEN = "\033[92m";
const std::string LogProcessor::RED = "\033[91m";
const std::string LogProcessor::YELLOW = "\033[93m";
const std::string LogProcessor::RESET = "\033[0m";


int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Using method: " << argv[0] << " <input_file>" << std::endl;
        return 1;
    }

    std::string input_file_path = argv[1];
    // Defaults from the Python script
    int year_to_use = 2025;
    std::string replacement_map_file = "end2duration.json";
    
    std::string input_data;
    std::ifstream input_file_stream(input_file_path);
    if (!input_file_stream.is_open()) {
        std::cerr << "错误: 输入文件 '" << input_file_path << "' 未找到或无法打开。" << std::endl;
        return 1;
    }

    input_file_stream.seekg(0, std::ios::end);
    input_data.reserve(input_file_stream.tellg());
    input_file_stream.seekg(0, std::ios::beg);
    input_data.assign((std::istreambuf_iterator<char>(input_file_stream)),
                       std::istreambuf_iterator<char>());
    input_file_stream.close();

    std::filesystem::path p(input_file_path);
    std::string base_input_filename = p.filename().string();
    std::string actual_output_filename = "processed_" + base_input_filename;

    std::cout << "Input file: " << input_file_path << std::endl;
    std::cout << "Output file saved to: " << actual_output_filename << std::endl;

    std::ofstream output_file_obj(actual_output_filename);
    if (!output_file_obj.is_open()) {
        std::cerr << "错误: 无法创建或打开输出文件 '" << actual_output_filename << "'。" << std::endl;
        return 1;
    }

    // Pass the ofstream object to LogProcessor, and true for is_file
    LogProcessor processor(year_to_use, replacement_map_file, output_file_obj, true);

    auto start_time = std::chrono::high_resolution_clock::now();
    
    try {
        processor.process_log_data(input_data);
    } catch (const std::exception& e) {
        std::cerr << "Error occurred,when processing file: " << e.what() << std::endl;
        output_file_obj.close(); // Ensure file is closed on error
        return 1;
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> processing_duration = end_time - start_time;

    output_file_obj.close(); // Close the file stream

    std::cout << "Processing complete,output saved to: " << actual_output_filename << std::endl;
    std::cout << std::fixed << std::setprecision(6) << "Time taken to process input content " << processing_duration.count() << " s" << std::endl;
    std::cout << "------------------------------" << std::endl;

    return 0;
}