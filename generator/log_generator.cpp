#include <iostream>
#include <vector>
#include <string>
#include <iomanip> // For std::setw, std::setfill
#include <sstream> // For std::ostringstream, std::to_string
#include <fstream> // For std::ofstream
#include <stdexcept> // For std::invalid_argument, std::out_of_range

// For Windows-specific console codepage setting
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

// Helper function to format a number to two digits with a leading zero
std::string format_two_digits(int n) {
    std::ostringstream ss;
    ss << std::setw(2) << std::setfill('0') << n;
    return ss.str();
}

void print_usage(const char* prog_name) {
    std::cerr << "Usage: " << prog_name << " <num_days> <items_per_day>" << std::endl;
    std::cerr << "Description: Generates test log data." << std::endl;
    std::cerr << "  <num_days>        : Total number of days to generate (positive integer)." << std::endl;
    std::cerr << "  <items_per_day>   : Number of log items per day (positive integer)." << std::endl;
    std::cerr << "Example: " << prog_name << " 10 5" << std::endl;
}

int main(int argc, char* argv[]) {
#if defined(_WIN32) || defined(_WIN64)
    // Attempt to set console output to UTF-8 on Windows.
    SetConsoleOutputCP(CP_UTF8); // CP_UTF8 is 65001
#endif

    if (argc != 3) {
        print_usage(argv[0]);
        return 1;
    }

    int num_days_val;
    int items_per_day_val;

    try {
        num_days_val = std::stoi(argv[1]);
        items_per_day_val = std::stoi(argv[2]);
    } catch (const std::invalid_argument& ia) {
        std::cerr << "Error: Invalid argument. <num_days> and <items_per_day> must be integers." << std::endl;
        std::cerr << ia.what() << std::endl;
        print_usage(argv[0]);
        return 1;
    } catch (const std::out_of_range& oor) {
        std::cerr << "Error: Argument out of range." << std::endl;
        std::cerr << oor.what() << std::endl;
        print_usage(argv[0]);
        return 1;
    }

    if (num_days_val <= 0 || items_per_day_val <= 0) {
        std::cerr << "Error: <num_days> and <items_per_day> must be positive integers." << std::endl;
        print_usage(argv[0]);
        return 1;
    }

    // --- Configuration (start date can be adjusted here if needed) ---
    int start_month = 1;
    int start_day_of_month = 1; // Defaulting to Jan 1st
    // --- End Configuration ---

    // Construct output filename
    std::ostringstream filename_ss;
    filename_ss << "log_" << num_days_val
                << "_items_" << items_per_day_val << ".txt";
    std::string output_filename_str = filename_ss.str();

    // Ensure start_month and start_day_of_month are somewhat sensible for the start.
    if (start_month < 1) start_month = 1;
    if (start_month > 12) start_month = 12;
    if (start_day_of_month < 1) start_day_of_month = 1;
    if (start_day_of_month > 31) start_day_of_month = 31; // Max possible, specific month length handled below

    int current_month = start_month;
    int current_day_of_month = start_day_of_month;

    // IMPORTANT: Ensure this source file is saved with UTF-8 encoding
    // for the Chinese characters to be correctly represented in string literals.
    std::vector<std::string> common_activities = {
        "word",         
        "饭中",         
        "timemaster",   
        "休息短",       
        "听力",         
        "bili",         
        "运动",         
        "洗澡",         
        "refactor",     
        "睡觉",          
        "单词",
        "听力",
        "文章",
        "timemaster",
        "refactor",
        "休息短",
        "休息中",
        "休息长",
        "洗澡",
        "快递",
        "洗漱",
        "拉屎",
        "饭短",
        "饭中",
        "饭长",
        "zh",
        "知乎",
        "dy",
        "抖音",
        "守望先锋",
        "皇室",
        "ow",
        "bili",
        "mix",
        "b",
        "电影",
        "撸",
        "school",
        "有氧",
        "无氧",
        "运动",
        "break"
    };

    std::ofstream outFile(output_filename_str);
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open file '" << output_filename_str << "' for writing." << std::endl;
        return 1;
    }

    std::cout << "Generating data to '" << output_filename_str << "'..." << std::endl;

    for (int d = 0; d < num_days_val; ++d) {
        // Print date line (MMDD)
        outFile << format_two_digits(current_month) << format_two_digits(current_day_of_month) << std::endl;

        int current_hour = 7;    // Start hour for events on this day
        int current_minute = 0;  // Start minute for events on this day

        for (int i = 0; i < items_per_day_val; ++i) {
            // Print event line (HHMMevent_text)
            outFile << format_two_digits(current_hour) << format_two_digits(current_minute);

            std::string event_text_to_use;
            if (i == 0) {
                // The log_processor checks for "醒" or "起床" for the first event.
                event_text_to_use = "起床";
            } else {
                if (!common_activities.empty()) {
                    // Cycle through common_activities for subsequent events
                    event_text_to_use = common_activities[(i - 1) % common_activities.size()];
                } else {
                    event_text_to_use = "generic_activity"; // Fallback if common_activities is empty
                }
            }
            outFile << event_text_to_use << std::endl;

            // Increment time for the next event
            current_minute += 45; // e.g., increment by 45 minutes
            if (current_minute >= 60) {
                current_hour += current_minute / 60;
                current_minute %= 60;
            }
            if (current_hour >= 24) {
                current_hour %= 24;
            }
        }

        // Advance to the next day
        current_day_of_month++;
        int days_in_current_month = 31; // Default
        if (current_month == 2) {
            days_in_current_month = 28; // Simplified: not accounting for leap years
        } else if (current_month == 4 || current_month == 6 || current_month == 9 || current_month == 11) {
            days_in_current_month = 30;
        }

        if (current_day_of_month > days_in_current_month) {
            current_day_of_month = 1;
            current_month++;
            if (current_month > 12) {
                current_month = 1; // Reset month
            }
        }

        // Add a blank line between day blocks for readability in the generated file.
        if (d < num_days_val - 1) {
            outFile << std::endl;
        }
    }

    outFile.close();
    std::cout << "Data generation complete. Output is in '" << output_filename_str << "'" << std::endl;

    return 0;
}
