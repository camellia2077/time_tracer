#include <iostream>
#include <vector>
#include <string>
#include <iomanip>     // For std::setw, std::setfill
#include <sstream>     // For std::ostringstream, std::to_string
#include <fstream>     // For std::ofstream, std::ifstream
#include <stdexcept>   // For std::invalid_argument, std::out_of_range
#include <random>      // For random number generation
#include <cmath>       // For std::round
#include <memory>      // For std::unique_ptr

// Include the nlohmann/json library.
// Make sure json.hpp is in your include path or in the same directory.
// If you placed it in a "nlohmann" subdirectory, use "nlohmann/json.hpp"
#include "json.hpp" // Assuming json.hpp is in the same directory or accessible via include paths

// For Windows-specific console codepage setting
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

// Use nlohmann::json
using json = nlohmann::json;

// Helper function to format a number to two digits with a leading zero
std::string format_two_digits(int n) {
    std::ostringstream ss;
    ss << std::setw(2) << std::setfill('0') << n;
    return ss.str();
}

void print_usage(const char* prog_name) {
    std::cerr << "Usage: " << prog_name << " <num_days> <items_per_day>" << std::endl;
    std::cerr << "Description: Generates test log data. Reads activities from 'activities_config.json'." << std::endl;
    std::cerr << "  <num_days>        : Total number of days to generate (positive integer)." << std::endl;
    std::cerr << "  <items_per_day>   : Number of log items per day (positive integer)." << std::endl;
    std::cerr << "Example: " << prog_name << " 10 5" << std::endl;
}

// Function to load common activities from a JSON file
std::vector<std::string> load_activities_from_json(const std::string& json_filename, const std::vector<std::string>& default_activities) {
    std::ifstream f(json_filename);
    if (!f.is_open()) {
        std::cerr << "Warning: Could not open activities configuration file '" << json_filename << "'. Using default activities." << std::endl;
        return default_activities;
    }

    try {
        json data = json::parse(f);
        f.close(); // Close the file stream after parsing

        if (data.contains("common_activities") && data["common_activities"].is_array()) {
            std::cout << "Successfully loaded activities from '" << json_filename << "'." << std::endl;
            return data["common_activities"].get<std::vector<std::string>>();
        } else {
            std::cerr << "Warning: JSON file '" << json_filename << "' does not contain a 'common_activities' array or it's not an array. Using default activities." << std::endl;
            return default_activities;
        }
    } catch (json::parse_error& e) {
        std::cerr << "Warning: Failed to parse JSON from '" << json_filename << "'. Error: " << e.what() << ". Using default activities." << std::endl;
        if(f.is_open()) f.close();
        return default_activities;
    } catch (json::type_error& e) {
        std::cerr << "Warning: JSON type error in '" << json_filename << "'. Is 'common_activities' an array of strings? Error: " << e.what() << ". Using default activities." << std::endl;
        if(f.is_open()) f.close();
        return default_activities;
    } catch (const std::exception& e) { // Catch other potential exceptions
        std::cerr << "Warning: An unexpected error occurred while reading or parsing '" << json_filename << "'. Error: " << e.what() << ". Using default activities." << std::endl;
        if(f.is_open()) f.close();
        return default_activities;
    }
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
     if (items_per_day_val < 2 && num_days_val > 0) {
        std::cerr << "Error: <items_per_day> must be at least 2 to include '起床' and '睡觉长'." << std::endl;
        return 1;
    }


    int start_month = 1;
    int start_day_of_month = 1;

    std::ostringstream filename_ss;
    // Using the filename from the third provided C++ file, without ".txt" initially
    filename_ss << "log_" << num_days_val
                << "_items_" << items_per_day_val; 
    std::string output_filename_str = filename_ss.str();


    if (start_month < 1) start_month = 1;
    if (start_month > 12) start_month = 12;
    if (start_day_of_month < 1) start_day_of_month = 1;
    if (start_day_of_month > 31) start_day_of_month = 31;

    int current_month = start_month;
    int current_day_of_month = start_day_of_month;

    // Define default activities as a fallback
    std::vector<std::string> default_common_activities = {
        "word_default", "饭中_default", "timemaster_default", "休息短_default", "听力_default", 
        "bili_default", "运动_default", "洗澡_default", "refactor_default", "单词_default"
        // Add more defaults if needed
    };

    // Load activities from JSON, or use defaults if loading fails
    std::vector<std::string> common_activities = load_activities_from_json("activities_config.json", default_common_activities);
    
    if (common_activities.empty() && items_per_day_val > 2) {
         std::cerr << "Warning: No common activities loaded or defined, and intermediate items are expected. Intermediate items will use 'generic_activity'." << std::endl;
    }


    std::ofstream outFile(output_filename_str); // Filename from third C++ example, no .txt here
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open file '" << output_filename_str << "' for writing." << std::endl;
        return 1;
    }

    std::cout << "Generating data to '" << output_filename_str << "'..." << std::endl;

    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> dis_minute(0, 59);
    // The dis_small_minute_offset from the first version of C++ code is not present in the third one.
    // I'll use the logic from the third C++ file which is simpler for intermediate items.
    // If dis_small_minute_offset is needed, it can be re-added.

    std::unique_ptr<std::uniform_int_distribution<>> dis_activity_selector;
    if (!common_activities.empty()) {
        dis_activity_selector = std::make_unique<std::uniform_int_distribution<>>(0, static_cast<int>(common_activities.size()) - 1);
    }

    int minute_for_todays_actual_wakeup = dis_minute(gen);

    // Using the logic from the third C++ file provided (labeled log_generator.cpp)
    // as it seems to be the most recent or intended version for modification.
    for (int d = 0; d < num_days_val; ++d) {
        outFile << format_two_digits(current_month) << format_two_digits(current_day_of_month) << std::endl;

        int minute_for_next_days_scheduled_wakeup = dis_minute(gen);

        // The time-ordering logic from the first C++ example (prev_event_logical_hour, etc.)
        // is not present in the third example. I'm adhering to the third example's structure here.
        // If strict time ordering for intermediate items is needed, that logic would need to be merged.
        // For this request, the focus is JSON loading.

        for (int i = 0; i < items_per_day_val; ++i) {
            int display_hour_final;
            int event_minute_final;
            std::string event_text_to_use_final;

            if (i == items_per_day_val - 1) { // LAST item
                event_text_to_use_final = "睡觉长";
                display_hour_final = 6; 
                event_minute_final = minute_for_next_days_scheduled_wakeup; 
            } else if (i == 0) { // FIRST item
                event_text_to_use_final = "起床";
                display_hour_final = 6;
                event_minute_final = minute_for_todays_actual_wakeup; 
            } else { // INTERMEDIATE items
                double progress_ratio = static_cast<double>(i) / (items_per_day_val - 1);
                int hour_offset = static_cast<int>(std::round(progress_ratio * 19.0));
                int logical_event_hour = 6 + hour_offset;
                
                if (logical_event_hour < 6) logical_event_hour = 6;
                if (logical_event_hour > 25) logical_event_hour = 25;

                if (logical_event_hour == 24) { 
                    display_hour_final = 0;
                } else if (logical_event_hour == 25) { 
                    display_hour_final = 1;
                } else { 
                    display_hour_final = logical_event_hour;
                }

                event_minute_final = dis_minute(gen); 

                if (dis_activity_selector) { 
                    event_text_to_use_final = common_activities[(*dis_activity_selector)(gen)];
                } else {
                    event_text_to_use_final = "generic_activity"; 
                }
            }
            outFile << format_two_digits(display_hour_final) << format_two_digits(event_minute_final) << event_text_to_use_final << std::endl;
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
