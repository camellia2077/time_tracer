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
    SetConsoleOutputCP(CP_UTF8);
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
     if (items_per_day_val < 2 && num_days_val > 0) { // Need at least "起床" and "睡觉长"
        std::cerr << "Error: <items_per_day> must be at least 2 to include '起床' and '睡觉长'." << std::endl;
        return 1;
    }


    int start_month = 1;
    int start_day_of_month = 1;

    std::ostringstream filename_ss;
    filename_ss << "log_" << num_days_val
                << "_items_" << items_per_day_val << ".txt"; // Added .txt extension
    std::string output_filename_str = filename_ss.str();

    if (start_month < 1) start_month = 1;
    if (start_month > 12) start_month = 12;
    if (start_day_of_month < 1) start_day_of_month = 1;
    if (start_day_of_month > 31) start_day_of_month = 31;

    int current_month = start_month;
    int current_day_of_month = start_day_of_month;

    std::vector<std::string> common_activities = {
        "word", "饭中", "timemaster", "休息短", "听力", "bili", "运动", "洗澡",
        "refactor", "单词", "听力", "文章", "timemaster", "refactor", "休息短",
        "休息中", "休息长", "洗澡", "快递", "洗漱", "拉屎", "饭短", "饭中", "饭长",
        "zh", "知乎", "dy", "抖音", "守望先锋", "皇室", "ow", "bili", "mix",
        "b", "电影", "撸", "school", "有氧", "无氧", "运动", "break"
    };
    if (items_per_day_val == 0 && !common_activities.empty()) { // Should not happen due to earlier check
         //This case is to prevent empty vector access if items_per_day_val could be 0,
         //but items_per_day_val <=0 is already an error.
    }


    std::ofstream outFile(output_filename_str);
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open file '" << output_filename_str << "' for writing." << std::endl;
        return 1;
    }

    std::cout << "Generating data to '" << output_filename_str << "'..." << std::endl;

    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> dis_minute(0, 59);
    std::uniform_int_distribution<> dis_small_minute_offset(0, 9); // For small minute variations after overflow

    std::unique_ptr<std::uniform_int_distribution<>> dis_activity_selector;
    if (!common_activities.empty()) {
        dis_activity_selector = std::make_unique<std::uniform_int_distribution<>>(0, static_cast<int>(common_activities.size()) - 1);
    }

    int minute_for_todays_actual_wakeup = dis_minute(gen);

    for (int d = 0; d < num_days_val; ++d) {
        outFile << format_two_digits(current_month) << format_two_digits(current_day_of_month) << std::endl;

        int minute_for_next_days_scheduled_wakeup = dis_minute(gen);

        int prev_event_logical_hour = -1; // Stores logical hour (6-25) of the previously generated event for the current day
        int prev_event_minute = -1;       // Stores minute (0-59) of the previously generated event

        for (int i = 0; i < items_per_day_val; ++i) {
            int final_display_hour;
            int final_event_minute;
            std::string event_text_to_use;
            int current_item_logical_hour; // The conceptual logical hour (6-25, or ~30 for 睡觉长)

            if (i == 0) { // FIRST item: "起床"
                event_text_to_use = "起床";
                current_item_logical_hour = 6; // Logical hour 6 AM
                final_event_minute = minute_for_todays_actual_wakeup;
                final_display_hour = 6;
            } else if (i == items_per_day_val - 1) { // LAST item: "睡觉长"
                event_text_to_use = "睡觉长";
                // This event's displayed time is fixed at 06:MM (next day's wakeup)
                // For internal comparison consistency, its logical hour is far in the future.
                current_item_logical_hour = 6 + 24; // Logical hour 30 (next day 6 AM)
                final_event_minute = minute_for_next_days_scheduled_wakeup;
                final_display_hour = 6; // Display hour is 06
            } else { // INTERMEDIATE items
                if (dis_activity_selector) {
                    event_text_to_use = common_activities[(*dis_activity_selector)(gen)];
                } else {
                    event_text_to_use = "generic_activity";
                }

                double progress_ratio = static_cast<double>(i) / (items_per_day_val - 1);
                int hour_offset = static_cast<int>(std::round(progress_ratio * 19.0)); // Original spread of 19 hours
                int target_logical_hour = 6 + hour_offset; // Tentative logical hour (approx 6 to 25)

                // Ensure current event's logical hour is not earlier than the previous event's logical hour.
                // prev_event_logical_hour is guaranteed to be set by "起床" if i > 0.
                if (target_logical_hour < prev_event_logical_hour) {
                    target_logical_hour = prev_event_logical_hour;
                }

                // Clamp intermediate events to a max logical hour (e.g., 25 for 1 AM next day)
                if (target_logical_hour > 25) target_logical_hour = 25;
                // Ensure it doesn't go below 6 (should be prevented by prev_event_logical_hour logic)
                if (target_logical_hour < 6) target_logical_hour = 6;


                current_item_logical_hour = target_logical_hour;
                final_event_minute = dis_minute(gen); // Generate a random minute

                // Adjust minute if necessary based on the previous event's time
                if (current_item_logical_hour == prev_event_logical_hour) {
                    if (final_event_minute <= prev_event_minute) {
                        final_event_minute = prev_event_minute + 1;
                    }
                }
                // Handle minute overflow if it was adjusted
                if (final_event_minute > 59) {
                    final_event_minute = dis_small_minute_offset(gen); // Start with a small random minute in the new hour
                    current_item_logical_hour++;
                    // Re-clamp if hour increment pushed it too far (e.g. for last intermediate item)
                    if (current_item_logical_hour > 25) current_item_logical_hour = 25;
                }
                
                // Convert current_item_logical_hour (6-25) to final_display_hour (0-23)
                if (current_item_logical_hour == 24) {
                    final_display_hour = 0;
                } else if (current_item_logical_hour == 25) {
                    final_display_hour = 1;
                } else { // Hours 6 to 23
                    final_display_hour = current_item_logical_hour;
                }
            }

            // Update trackers with the time of the event we are about to write
            prev_event_logical_hour = current_item_logical_hour;
            prev_event_minute = final_event_minute;

            outFile << format_two_digits(final_display_hour) << format_two_digits(final_event_minute) << event_text_to_use << std::endl;
        }

        minute_for_todays_actual_wakeup = minute_for_next_days_scheduled_wakeup;

        current_day_of_month++;
        int days_in_current_month = 31;
        if (current_month == 2) {
            days_in_current_month = 28;
        } else if (current_month == 4 || current_month == 6 || current_month == 9 || current_month == 11) {
            days_in_current_month = 30;
        }

        if (current_day_of_month > days_in_current_month) {
            current_day_of_month = 1;
            current_month++;
            if (current_month > 12) {
                current_month = 1;
            }
        }

        if (d < num_days_val - 1) {
            outFile << std::endl;
        }
    }

    outFile.close();
    std::cout << "Data generation complete. Output is in '" << output_filename_str << "'" << std::endl;

    return 0;
}
