#include "cli/CommandLineParser.h"
#include "common/AnsiColors.hpp" // [核心修改] 引入新的颜色头文件
#include "version.h"
#include <iostream>
#include <string>
#include <stdexcept>

// ... 构造函数和 version_requested/print_version 不变 ...
CommandLineParser::CommandLineParser(int argc, char* argv[])
    : argc_(argc), argv_(argv), prog_name_(argv[0]) {}

bool CommandLineParser::version_requested() const {
    return argc_ == 2 && std::string(argv_[1]) == "--version";
}

void CommandLineParser::print_version() const {
    std::cout << "log_generator version " << AppVersion::APP_VERSION << std::endl;
    std::cout << "Last Updated: " << AppVersion::LAST_UPDATE << std::endl;
}


void CommandLineParser::print_usage() const {
    // [核心修改] 使用新的颜色宏
    std::cerr << RED_COLOR << "Usage: " << RESET_COLOR << prog_name_ << " <start_year> <end_year> <items_per_day>\n";
    std::cerr << "       " << prog_name_ << " --version\n";
    std::cerr << "Description: Generates test log data for a given year range. Reads activities from 'activities_config.json'.\n";
    std::cerr << "  <start_year>      : The starting year (e.g., 1990).\n";
    std::cerr << "  <end_year>        : The ending year (inclusive).\n";
    std::cerr << "  <items_per_day>   : Number of log items per day (must be >= 2).\n";
    std::cerr << "  --version         : Display version information and exit.\n";
    std::cerr << "Example: " << prog_name_ << " 2023 2024 5\n";
}

std::optional<Config> CommandLineParser::parse() {
    if (argc_ != 4) {
        print_usage();
        return std::nullopt;
    }

    Config config;
    try {
        config.start_year = std::stoi(argv_[1]);
        config.end_year = std::stoi(argv_[2]);
        config.items_per_day = std::stoi(argv_[3]);
    }
    catch (const std::invalid_argument&) {
        // [核心修改] 使用新的颜色宏
        std::cerr << RED_COLOR << "Error" << RESET_COLOR << ": Invalid argument. All arguments must be integers.\n";
        print_usage();
        return std::nullopt;
    }
    catch (const std::out_of_range&) {
        // [核心修改] 使用新的颜色宏
        std::cerr << RED_COLOR << "Error" << RESET_COLOR <<": Argument out of range.\n";
        print_usage();
        return std::nullopt;
    }

    if (config.start_year <= 0 || config.end_year <= 0) {
        // [核心修改] 使用新的颜色宏
        std::cerr <<  RED_COLOR << "Error" << RESET_COLOR <<": Years must be positive integers.\n";
        print_usage();
        return std::nullopt;
    }
    
    if (config.items_per_day < 2) {
        // [核心修改] 使用新的颜色宏
        std::cerr << RED_COLOR << "Error" << RESET_COLOR <<": <items_per_day> must be 2 or greater to generate realistic sleep data.\n";
        std::cerr << "       (A value of 1 would make 'getup' the last event of the day).\n";
        print_usage();
        return std::nullopt;
    }

    if (config.end_year < config.start_year) {
        // [核心修改] 使用新的颜色宏
        std::cerr << RED_COLOR << "Error" << RESET_COLOR << ": <end_year> cannot be earlier than <start_year>.\n";
        print_usage();
        return std::nullopt;
    }

    return config;
}