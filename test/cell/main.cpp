#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <numeric> // For std::accumulate
#include <regex>

// --- Data Structures (from time_sheet_model.h) ---
struct DayData {
    std::string date;
    std::string status;
    std::string sleep;
    std::string remark;
    std::string getup_time;
};

struct TimeRecordInternal {
    std::string date;
    std::string start;
    std::string end;
    std::string project_path;
    int duration_seconds;
};

// --- Color Codes for Output ---
const std::string RED_COLOR = "\033[1;31m";
const std::string GREEN_COLOR = "\033[1;32m";
const std::string YELLOW_COLOR = "\033[1;33m";
const std::string BLUE_COLOR = "\033[1;34m";
const std::string MAGENTA_COLOR = "\033[1;35m";
const std::string CYAN_COLOR = "\033[1;36m";
const std::string RESET_COLOR = "\033[0m";

// --- Helper Functions (Simulating dependencies) ---
int time_str_to_seconds(const std::string& time_str) {
    if (time_str.length() != 5 || time_str[2] != ':') {
        return 0; // Invalid format
    }
    try {
        int hours = std::stoi(time_str.substr(0, 2));
        int minutes = std::stoi(time_str.substr(3, 2));
        return hours * 3600 + minutes * 60;
    } catch (const std::exception& e) {
        return 0; // Conversion error
    }
}

// --- Global variables for processing state ---
std::vector<DayData> processed_days;
std::vector<TimeRecordInternal> processed_records;
DayData current_day_data;
std::vector<TimeRecordInternal> buffered_records_for_day;

// --- Processing Logic Functions (from DataFileParser.cpp) ---

// 保存前一天的所有缓冲数据
void store_previous_date_data() {
    if (current_day_data.date.empty()) {
        return; // 如果没有当前日期，则不执行任何操作
    }
    processed_days.push_back(current_day_data);
    processed_records.insert(processed_records.end(), buffered_records_for_day.begin(), buffered_records_for_day.end());
    
    // 清空缓冲区以便处理下一天的数据
    buffered_records_for_day.clear();
    current_day_data = {}; // 重置
}

void handle_date_line(const std::string& line) {
    store_previous_date_data(); // 保存之前一天的数据
    current_day_data.date = line.substr(5); // "Date:" is 5 chars
    // 设置新日期的默认值
    current_day_data.status = "False";
    current_day_data.sleep = "False";
    current_day_data.remark = "";
    current_day_data.getup_time = "00:00";
}

void handle_status_line(const std::string& line) {
    current_day_data.status = line.substr(7); // "Status:" is 7 chars
}

void handle_sleep_line(const std::string& line) {
    current_day_data.sleep = line.substr(6); // "Sleep:" is 6 chars
}

void handle_remark_line(const std::string& line) {
    current_day_data.remark = line.substr(7); // "Remark:" is 7 chars
}

void handle_getup_line(const std::string& line) {
    current_day_data.getup_time = line.substr(6); // "Getup:" is 6 chars
}

void handle_time_record_line(const std::string& line) {
    if (current_day_data.date.empty()) return; // 必须在有日期的前提下处理

    static const std::regex time_record_regex(R"(([0-9]{2}:[0-9]{2})~([0-9]{2}:[0-9]{2})(.+))");
    std::smatch matches;
    if (std::regex_match(line, matches, time_record_regex) && matches.size() == 4) {
        std::string start_time_str = matches[1].str();
        std::string end_time_str = matches[2].str();
        std::string project_path = matches[3].str();
        
        // 去除 project_path 前后的多余空白
        project_path.erase(0, project_path.find_first_not_of(" \t_"));
        project_path.erase(project_path.find_last_not_of(" \t_") + 1);

        int start_seconds = time_str_to_seconds(start_time_str);
        int end_seconds = time_str_to_seconds(end_time_str);
        int duration_seconds = (end_seconds < start_seconds) 
                               ? ((end_seconds + 24 * 3600) - start_seconds) 
                               : (end_seconds - start_seconds);

        buffered_records_for_day.push_back({current_day_data.date, start_time_str, end_time_str, project_path, duration_seconds});
    }
}


// --- Classification and Printing Functions ---
void classify_and_process_line(const std::string& line) {
    auto trimmed_line = line;
    trimmed_line.erase(0, trimmed_line.find_first_not_of(" \t\n\r\f\v"));
    trimmed_line.erase(trimmed_line.find_last_not_of(" \t\n\r\f\v") + 1);

    if (trimmed_line.empty()) return;

    if (trimmed_line.rfind("Date:", 0) == 0) {
        handle_date_line(trimmed_line);
    } else if (trimmed_line.rfind("Status:", 0) == 0) {
        handle_status_line(trimmed_line);
    } else if (trimmed_line.rfind("Sleep:", 0) == 0) {
        handle_sleep_line(trimmed_line);
    } else if (trimmed_line.rfind("Remark:", 0) == 0) {
        handle_remark_line(trimmed_line);
    } else if (trimmed_line.rfind("Getup:", 0) == 0) {
        handle_getup_line(trimmed_line);
    } else if (trimmed_line.find('~') != std::string::npos) {
        handle_time_record_line(trimmed_line);
    }
}

void print_section(const std::string& title, const std::vector<std::string>& lines, const std::string& color) {
    if (!lines.empty()) {
        std::cout << color << "--- " << title << " ---" << RESET_COLOR << std::endl;
        for (const auto& line : lines) {
            std::cout << line << std::endl;
        }
        std::cout << std::endl;
    }
}

void print_processed_results() {
    std::cout << MAGENTA_COLOR << "################################" << std::endl;
    std::cout << "---    Processed Results     ---" << std::endl;
    std::cout << "################################" << RESET_COLOR << std::endl << std::endl;

    for (const auto& day : processed_days) {
        std::cout << BLUE_COLOR << "Date: " << RESET_COLOR << day.date << std::endl;
        std::cout << "  - Status: " << day.status << std::endl;
        std::cout << "  - Sleep: " << day.sleep << std::endl;
        std::cout << "  - Getup: " << day.getup_time << std::endl;
        std::cout << "  - Remark: " << day.remark << std::endl;
        std::cout << GREEN_COLOR << "  - Time Records:" << RESET_COLOR << std::endl;
        bool has_records_for_day = false;
        for (const auto& record : processed_records) {
            if (record.date == day.date) {
                has_records_for_day = true;
                std::cout << "    - Start: " << record.start 
                          << ", End: " << record.end 
                          << ", Duration: " << record.duration_seconds / 60.0 << " mins"
                          << ", Project: " << record.project_path << std::endl;
            }
        }
        if (!has_records_for_day) {
            std::cout << "    (No time records for this day)" << std::endl;
        }
        std::cout << std::endl;
    }
}


int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << RED_COLOR << "使用方法: " << argv[0] << " <文件名>" << RESET_COLOR << std::endl;
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file.is_open()) {
        std::cerr << RED_COLOR << "错误: 无法打开文件 " << argv[1] << RESET_COLOR << std::endl;
        return 1;
    }

    // ========== Part 1: Classification and Printing (as before) ==========
    std::vector<std::string> all_lines;
    std::string line;
    while (std::getline(file, line)) {
        all_lines.push_back(line);
    }
    file.clear();
    file.seekg(0, std::ios::beg); // Reset file stream to beginning

    // Classify
    std::vector<std::string> date_lines, status_lines, sleep_lines, remark_lines, getup_lines, time_record_lines, other_lines;
    for (const auto& l : all_lines) {
        auto trimmed = l;
        trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r\f\v"));
        trimmed.erase(trimmed.find_last_not_of(" \t\n\r\f\v") + 1);
        if (trimmed.empty()) continue;
        if (trimmed.rfind("Date:", 0) == 0) date_lines.push_back(trimmed);
        else if (trimmed.rfind("Status:", 0) == 0) status_lines.push_back(trimmed);
        else if (trimmed.rfind("Sleep:", 0) == 0) sleep_lines.push_back(trimmed);
        else if (trimmed.rfind("Remark:", 0) == 0) remark_lines.push_back(trimmed);
        else if (trimmed.rfind("Getup:", 0) == 0) getup_lines.push_back(trimmed);
        else if (trimmed.find('~') != std::string::npos) time_record_lines.push_back(trimmed);
        else other_lines.push_back(trimmed);
    }

    // Print classified sections
    std::cout << YELLOW_COLOR << "========================================" << std::endl;
    std::cout << "---         Phase 1: Raw         ---" << std::endl;
    std::cout << "========================================" << RESET_COLOR << std::endl << std::endl;

    print_section("Date Lines", date_lines, GREEN_COLOR);
    print_section("Status Lines", status_lines, YELLOW_COLOR);
    print_section("Sleep Lines", sleep_lines, CYAN_COLOR);
    print_section("Remark Lines", remark_lines, MAGENTA_COLOR);
    print_section("Getup Lines", getup_lines, BLUE_COLOR);
    print_section("Time Record Lines", time_record_lines, GREEN_COLOR);
    print_section("Other Lines", other_lines, RED_COLOR);

    // ========== Part 2: Processing and Printing Results ==========
    while (std::getline(file, line)) {
        classify_and_process_line(line);
    }
    store_previous_date_data(); // 确保最后一个日期的数据被保存

    print_processed_results();

    return 0;
}