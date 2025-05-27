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
                << "_items_" << items_per_day_val << "_final_sleep.txt"; // Updated filename
    std::string output_filename_str = filename_ss.str();

    // Ensure start_month and start_day_of_month are somewhat sensible for the start.
    if (start_month < 1) start_month = 1;
    if (start_month > 12) start_month = 12;
    if (start_day_of_month < 1) start_day_of_month = 1;
    if (start_day_of_month > 31) start_day_of_month = 31; // Max possible, specific month length handled below

    int current_month = start_month;
    int current_day_of_month = start_day_of_month;

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

    // Stores the wakeup minute for the *current* day's "起床" event.
    // Initialized for the first day (d=0).
    int minute_for_todays_actual_wakeup = dis_minute(gen);

    for (int d = 0; d < num_days_val; ++d) {
        // Print date line (MMDD)
        outFile << format_two_digits(current_month) << format_two_digits(current_day_of_month) << std::endl;

        // Determine the wakeup minute for the *next* day (d+1).
        // This will be used for the current day's (d) "睡觉长" item's time.
        // And it will become `minute_for_todays_actual_wakeup` in the next iteration.
        int minute_for_next_days_scheduled_wakeup = dis_minute(gen);

        for (int i = 0; i < items_per_day_val; ++i) {
            int display_hour_final;
            int event_minute_final;
            std::string event_text_to_use_final;

            if (i == items_per_day_val - 1) { // LAST item of the day
                event_text_to_use_final = "睡觉长";
                display_hour_final = 6; // Next day's wakeup hour is 06:00
                event_minute_final = minute_for_next_days_scheduled_wakeup; // Use (d+1)'s scheduled wakeup minute
            } else if (i == 0) { // FIRST item (and not the last, as that's covered above)
                event_text_to_use_final = "起床";
                display_hour_final = 6;
                event_minute_final = minute_for_todays_actual_wakeup; // Use (d)'s actual wakeup minute
            } else { // INTERMEDIATE items
                // These items are between "起床" and the item just before "睡觉长".
                // Their times should be distributed within the 06:00 current day to ~01:00 next day (current cycle).
                double progress_ratio = static_cast<double>(i) / (items_per_day_val - 1);
                int hour_offset = static_cast<int>(std::round(progress_ratio * 19.0));
                int logical_event_hour = 6 + hour_offset;
                
                // Ensure logical_event_hour stays within the 6 to 25 range (defensive)
                if (logical_event_hour < 6) logical_event_hour = 6;
                if (logical_event_hour > 25) logical_event_hour = 25;


                if (logical_event_hour == 24) { // Midnight
                    display_hour_final = 0;
                } else if (logical_event_hour == 25) { // 1 AM next day
                    display_hour_final = 1;
                } else { // Hours 6 to 23
                    display_hour_final = logical_event_hour;
                }

                event_minute_final = dis_minute(gen); // New random minute for this intermediate item

                if (dis_activity_selector) { // If common_activities is not empty
                    event_text_to_use_final = common_activities[(*dis_activity_selector)(gen)];
                } else {
                    event_text_to_use_final = "generic_activity"; // Fallback
                }
            }
            outFile << format_two_digits(display_hour_final) << format_two_digits(event_minute_final) << event_text_to_use_final << std::endl;
        }

        // For the next day (d+1), its "起床" minute will be what we just scheduled for its wakeup.
        minute_for_todays_actual_wakeup = minute_for_next_days_scheduled_wakeup;

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
