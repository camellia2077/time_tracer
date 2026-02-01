// cli/framework/command_line_parser.cpp
#include "cli/framework/command_line_parser.hpp"

#include <stdexcept>

CommandLineParser::CommandLineParser(int argc, char* argv[]) {
  if (argc > 0 && argv != nullptr) {
    prog_name_ = argv[0];
  }
  for (int i = 1; i < argc; ++i) {
    args_.emplace_back(argv[i]);
  }
}

auto CommandLineParser::parse() -> CliRequest {
  Config config;
  config.items_per_day = 10;

  std::optional<int> single_year;
  std::optional<int> start_year;
  std::optional<int> end_year;

  try {
    for (size_t i = 0; i < args_.size(); ++i) {
      const std::string& arg = args_[i];

      if (arg == "-h" || arg == "--help") {
        return CliRequest{.action = CliAction::kHelp,
                          .config = std::nullopt,
                          .error_message = {}};
      }
      if (arg == "-v" || arg == "--version") {
        return CliRequest{.action = CliAction::kVersion,
                          .config = std::nullopt,
                          .error_message = {}};
      }

      if (arg == "-y" || arg == "--year") {
        if (i + 1 < args_.size()) {
          single_year = std::stoi(args_[++i]);
        } else {
          throw std::invalid_argument("--year option requires an argument.");
        }
      } else if (arg == "-s" || arg == "--start") {
        if (i + 1 < args_.size()) {
          start_year = std::stoi(args_[++i]);
        } else {
          throw std::invalid_argument("--start option requires an argument.");
        }
      } else if (arg == "-e" || arg == "--end") {
        if (i + 1 < args_.size()) {
          end_year = std::stoi(args_[++i]);
        } else {
          throw std::invalid_argument("--end option requires an argument.");
        }
      } else if (arg == "-i" || arg == "--items") {
        if (i + 1 < args_.size()) {
          config.items_per_day = std::stoi(args_[++i]);
        } else {
          throw std::invalid_argument("--items option requires an argument.");
        }
      } else if (arg == "-n" || arg == "--nosleep") {
        config.enable_nosleep = true;
      } else {
        throw std::invalid_argument("Unrecognized option: " + arg);
      }
    }

    if (single_year.has_value() &&
        (start_year.has_value() || end_year.has_value())) {
      throw std::logic_error(
          "Cannot use --year together with --start or --end.");
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
      throw std::logic_error(
          "You must specify either a single year with --year, or a range with "
          "--start and --end.");
    }

    if (config.start_year <= 0 || config.end_year <= 0) {
      throw std::logic_error("Years must be positive integers.");
    }
    if (config.items_per_day < 2) {
      throw std::logic_error(
          "--items must be 2 or greater to generate realistic sleep data.");
    }
    if (config.end_year < config.start_year) {
      throw std::logic_error("--end year cannot be earlier than --start year.");
    }
  } catch (const std::invalid_argument& e) {
    return CliRequest{
        .action = CliAction::kError,
        .config = std::nullopt,
        .error_message = "Invalid argument. " + std::string(e.what())};
  } catch (const std::out_of_range&) {
    return CliRequest{.action = CliAction::kError,
                      .config = std::nullopt,
                      .error_message = "Argument out of range."};
  } catch (const std::logic_error& e) {
    return CliRequest{.action = CliAction::kError,
                      .config = std::nullopt,
                      .error_message = e.what()};
  }

  return CliRequest{
      .action = CliAction::kRun, .config = config, .error_message = {}};
}

auto CommandLineParser::prog_name() const -> std::string_view {
  return prog_name_;
}
