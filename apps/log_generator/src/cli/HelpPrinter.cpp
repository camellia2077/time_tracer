#include "cli/HelpPrinter.h"
#include "common/AnsiColors.hpp"
#include "version.h" // 需要访问版本信息
#include <iostream>

HelpPrinter::HelpPrinter(const std::string& prog_name) : prog_name_(prog_name) {}

void HelpPrinter::print_version() const {
    std::cout << "log_generator version " << AppVersion::APP_VERSION << std::endl;
    std::cout << "Last Updated: " << AppVersion::LAST_UPDATE << std::endl;
}

void HelpPrinter::print_error(const std::string& message) const {
    std::cerr << RED_COLOR << "Error" << RESET_COLOR << ": " << message << "\n\n";
}

void HelpPrinter::print_usage() const {
    std::cerr << GREEN_COLOR << "Usage: " << RESET_COLOR << prog_name_ << " [options]\n\n";
    std::cerr << "Description: Generates test log data for a given year or year range.\n";
    std::cerr << "             Reads activity configurations from 'config/activities_config.json'.\n\n";
    std::cerr << GREEN_COLOR << "Options:\n" << RESET_COLOR;
    std::cerr << "  -y, --year <year>       Generate data for a single year.\n";
    std::cerr << "  -s, --start <year>      The starting year for a range. (Used with --end)\n";
    std::cerr << "  -e, --end <year>        The ending year for a range (inclusive). (Used with --start)\n";
    std::cerr << "  -i, --items <number>    Number of log items per day (must be >= 2). (Default: 10)\n";
    std::cerr << "  -n, --nosleep           Enable the generation of 'no sleep' (all-nighter) days.\n"; // 新增
    std::cerr << "  -v, --version           Display version information and exit.\n";
    std::cerr << "  -h, --help              Display this help message and exit.\n\n";
    std::cerr << GREEN_COLOR << "Example:\n" << RESET_COLOR;
    std::cerr << "  " << prog_name_ << " --year 2025\n";
    std::cerr << "  " << prog_name_ << " --start 2024 --end 2025 --items 5 --nosleep\n";
}