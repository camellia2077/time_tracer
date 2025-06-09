#include "common_utils.h"

// Helper function to convert HH:MM or HHMM time string to seconds since midnight
int time_str_to_seconds(const std::string& time_str_in) {
    std::string time_str = time_str_in;
    // Normalize HHMM to HH:MM if needed
    if (time_str.length() == 4 && time_str.find(':') == std::string::npos) {
        bool all_digits = true;
        for (char c : time_str) {
            if (!std::isdigit(static_cast<unsigned char>(c))) {
                all_digits = false;
                break;
            }
        }
        if (all_digits) {
            time_str = time_str.substr(0, 2) + ":" + time_str.substr(2, 2);
        }
    }

    if (time_str.length() != 5 || time_str[2] != ':') {
        if (time_str != "00:00") {
           // std::cerr << "Error: Invalid time format '" << time_str << "'. Expected HH:MM." << std::endl;
        }
        return 0;
    }
    try {
        int hours = std::stoi(time_str.substr(0, 2));
        int minutes = std::stoi(time_str.substr(3, 2));
        if (hours < 0 || hours > 23 || minutes < 0 || minutes > 59) {
            // std::cerr << "Error: Invalid time value in '" << time_str << "'." << std::endl;
            return 0;
        }
        return hours * 3600 + minutes * 60;
    } catch (const std::invalid_argument& ia) {
        // std::cerr << "Error: Non-numeric value in time '" << time_str << "': " << ia.what() << std::endl;
        return 0;
    } catch (const std::out_of_range& oor) {
        // std::cerr << "Error: Time value out of range in '" << time_str << "': " << oor.what() << std::endl;
        return 0;
    }
}

// Formats total seconds into a string like "XhYYm" or "YYm", with optional averaging.
std::string time_format_duration(long long total_seconds, int avg_days) {
    if (total_seconds == 0) {
        if (avg_days > 1) return "0m (average: 0m/day)";
        return "0m";
    }

    long long seconds_for_format = total_seconds;
    long long seconds_per_day = total_seconds;

    if (avg_days > 1) {
         seconds_per_day = total_seconds / avg_days;
    }

    auto format_single_duration = [](long long s) {
        if (s == 0) return std::string("0m");
        long long h = s / 3600;
        long long m = (s % 3600) / 60;
        std::stringstream ss;
        if (h > 0) {
            ss << h << "h";
        }
        if (m > 0 || h == 0) {
            ss << m << "m";
        }
        return ss.str();
    };

    std::string main_duration_str = format_single_duration(seconds_for_format);

    if (avg_days > 1) {
        std::string avg_duration_str = format_single_duration(seconds_per_day);
        main_duration_str += " (average: " + avg_duration_str + "/day)";
    }
    return main_duration_str;
}

// Splits a string by a delimiter.
std::vector<std::string> split_string(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}