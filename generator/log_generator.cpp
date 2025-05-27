#include <iostream>
#include <vector>
#include <string>
#include <iomanip>     // For std::setw, std::setfill
#include <sstream>     // For std::ostringstream, std::to_string
#include <fstream>     // For std::ofstream
#include <stdexcept>   // For std::invalid_argument, std::out_of_range
#include <random>      // For random number generation
#include <cmath>       // For std::round
#include <memory>      // For std::unique_ptr

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
                << "_items_" << items_per_day_val << "_randomized.txt"; // Added _randomized to filename
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
        "word", "饭中", "timemaster", "休息短", "听力", "bili", "运动", "洗澡",
        "refactor", "单词", "听力", "文章", "timemaster", "refactor", "休息短",
        "休息中", "休息长", "洗澡", "快递", "洗漱", "拉屎", "饭短", "饭中", "饭长",
        "zh", "知乎", "dy", "抖音", "守望先锋", "皇室", "ow", "bili", "mix",
        "b", "电影", "撸", "school", "有氧", "无氧", "运动", "break"
    };

    std::ofstream outFile(output_filename_str);
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open file '" << output_filename_str << "' for writing." << std::endl;
        return 1;
    }

    std::cout << "Generating data to '" << output_filename_str << "'..." << std::endl;

    // Initialize random number generation tools
    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> dis_minute(0, 59); // For random minutes

    std::unique_ptr<std::uniform_int_distribution<>> dis_activity_selector;
    if (!common_activities.empty()) {
        dis_activity_selector = std::make_unique<std::uniform_int_distribution<>>(0, static_cast<int>(common_activities.size()) - 1);
    }

    for (int d = 0; d < num_days_val; ++d) {
        // Print date line (MMDD)
        outFile << format_two_digits(current_month) << format_two_digits(current_day_of_month) << std::endl;

        for (int i = 0; i < items_per_day_val; ++i) {
            int logical_event_hour;
            int event_minute = dis_minute(gen); // Random minute for this event
            std::string event_text_to_use;

            // Determine the logical hour for the event (6 AM to 1 AM next day, i.e., 6 to 25)
            // This distributes items_per_day_val items across a 19-hour span.
            if (items_per_day_val == 1) {
                logical_event_hour = 6; // Single item at 06:MM
            } else {
                // progress_ratio goes from 0 (for i=0) to 1 (for i=items_per_day_val-1)
                double progress_ratio = static_cast<double>(i) / (items_per_day_val - 1);
                // hour_offset goes from 0 to 19
                int hour_offset = static_cast<int>(std::round(progress_ratio * 19.0));
                logical_event_hour = 6 + hour_offset;
            }
            
            // Ensure logical_event_hour stays within the 6 to 25 range (defensive)
            if (logical_event_hour < 6) logical_event_hour = 6;
            if (logical_event_hour > 25) logical_event_hour = 25;


            int display_hour;
            if (logical_event_hour == 24) { // Midnight
                display_hour = 0;
            } else if (logical_event_hour == 25) { // 1 AM next day
                display_hour = 1;
            } else { // Hours 6 to 23
                display_hour = logical_event_hour;
            }

            outFile << format_two_digits(display_hour) << format_two_digits(event_minute);

            if (i == 0) {
                event_text_to_use = "起床";
            } else {
                if (dis_activity_selector) { // If common_activities is not empty
                    event_text_to_use = common_activities[(*dis_activity_selector)(gen)];
                } else {
                    event_text_to_use = "generic_activity"; // Fallback
                }
            }
            outFile << event_text_to_use << std::endl;
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
                current_month = 1; // Reset month (year is not tracked in this simplified version)
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
