#include "cli/CommandLineParser.h"
#include <iostream>
#include <stdexcept>

// [核心修改] 构造函数现在初始化 HelpPrinter
CommandLineParser::CommandLineParser(int argc, char* argv[])
    : printer_(argv[0]) { // 将程序名传递给 printer
    for (int i = 1; i < argc; ++i) {
        args_.push_back(argv[i]);
    }
}

// [核心修改] 移除 print_version 和 print_usage 的实现

std::optional<Config> CommandLineParser::parse() {
    Config config;
    config.items_per_day = 10;
    
    std::optional<int> single_year, start_year, end_year;

    try {
        for (size_t i = 0; i < args_.size(); ++i) {
            const std::string& arg = args_[i];

            if (arg == "-h" || arg == "--help") {
                printer_.print_usage(); // 委托给 printer
                return std::nullopt;
            }
            if (arg == "-v" || arg == "--version") {
                printer_.print_version(); // 委托给 printer
                return std::nullopt;
            }

            if (arg == "-y" || arg == "--year") {
                if (i + 1 < args_.size()) {
                    single_year = std::stoi(args_[++i]);
                } else { throw std::invalid_argument("--year option requires an argument."); }
            } else if (arg == "-s" || arg == "--start") {
                if (i + 1 < args_.size()) {
                    start_year = std::stoi(args_[++i]);
                } else { throw std::invalid_argument("--start option requires an argument."); }
            } else if (arg == "-e" || arg == "--end") {
                if (i + 1 < args_.size()) {
                    end_year = std::stoi(args_[++i]);
                } else { throw std::invalid_argument("--end option requires an argument."); }
            } else if (arg == "-i" || arg == "--items") {
                if (i + 1 < args_.size()) {
                    config.items_per_day = std::stoi(args_[++i]);
                } else { throw std::invalid_argument("--items option requires an argument."); }
            } else if (arg == "-n" || arg == "--nosleep") { // 新增：处理通宵选项
                config.enable_nosleep = true;
            }
            else {
                throw std::invalid_argument("Unrecognized option: " + arg);
            }
        }

        if (single_year.has_value() && (start_year.has_value() || end_year.has_value())) {
            throw std::logic_error("Cannot use --year together with --start or --end.");
        }

        if (single_year.has_value()) {
            config.mode = GenerationMode::SingleYear;
            config.start_year = *single_year;
            config.end_year = *single_year;
        } else if (start_year.has_value() && end_year.has_value()) {
            config.mode = GenerationMode::YearRange;
            config.start_year = *start_year;
            config.end_year = *end_year;
        } else {
            throw std::logic_error("You must specify either a single year with --year, or a range with --start and --end.");
        }

        if (config.start_year <= 0 || config.end_year <= 0) {
            throw std::logic_error("Years must be positive integers.");
        }
        if (config.items_per_day < 2) {
            throw std::logic_error("--items must be 2 or greater to generate realistic sleep data.");
        }
        if (config.end_year < config.start_year) {
            throw std::logic_error("--end year cannot be earlier than --start year.");
        }

    } catch (const std::invalid_argument& e) {
        printer_.print_error("Invalid argument. " + std::string(e.what())); // 委托给 printer
        printer_.print_usage();
        return std::nullopt;
    } catch (const std::out_of_range&) {
        printer_.print_error("Argument out of range."); // 委托给 printer
        printer_.print_usage();
        return std::nullopt;
    } catch (const std::logic_error& e) {
        printer_.print_error(e.what()); // 委托给 printer
        printer_.print_usage();
        return std::nullopt;
    }

    return config;
}