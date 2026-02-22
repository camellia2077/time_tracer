// cli/framework/command_line_parser.cpp
#include "cli/framework/command_line_parser.hpp"

#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {

constexpr int kDefaultItemsPerDay = 10;
constexpr int kMinItemsPerDay = 2;
constexpr int kMinYear = 1;

enum class OptionType {
  kHelp,
  kVersion,
  kYear,
  kStart,
  kEnd,
  kItems,
  kNoSleep,
  kMonthlyAverage,
  kOutput
};

struct ParsedOptions {
  Config config{};
  std::optional<int> single_year;
  std::optional<int> start_year;
  std::optional<int> end_year;
  std::optional<CliAction> immediate_action;
};

auto ClassifyOption(const std::string& arg) -> OptionType {
  if (arg == "-h" || arg == "--help") {
    return OptionType::kHelp;
  }
  if (arg == "-v" || arg == "--version") {
    return OptionType::kVersion;
  }
  if (arg == "-y" || arg == "--year") {
    return OptionType::kYear;
  }
  if (arg == "-s" || arg == "--start") {
    return OptionType::kStart;
  }
  if (arg == "-e" || arg == "--end") {
    return OptionType::kEnd;
  }
  if (arg == "-i" || arg == "--items") {
    return OptionType::kItems;
  }
  if (arg == "-n" || arg == "--nosleep") {
    return OptionType::kNoSleep;
  }
  if (arg == "--monthly-average") {
    return OptionType::kMonthlyAverage;
  }
  if (arg == "-o" || arg == "--output") {
    return OptionType::kOutput;
  }
  throw std::invalid_argument("Unrecognized option: " + arg);
}

auto ParseNextInt(const std::vector<std::string>& args, size_t& index,
                  std::string_view option_name) -> int {
  if (index + 1 >= args.size()) {
    throw std::invalid_argument(std::string(option_name) +
                                " option requires an argument.");
  }
  return std::stoi(args[++index]);
}

auto ParseNextString(const std::vector<std::string>& args, size_t& index,
                     std::string_view option_name) -> std::string {
  if (index + 1 >= args.size()) {
    throw std::invalid_argument(std::string(option_name) +
                                " option requires an argument.");
  }
  return args[++index];
}

auto ParseOptions(const std::vector<std::string>& args) -> ParsedOptions {
  ParsedOptions parsed;
  parsed.config.items_per_day = kDefaultItemsPerDay;

  for (size_t i = 0; i < args.size(); ++i) {
    const std::string& arg = args[i];
    switch (ClassifyOption(arg)) {
      case OptionType::kHelp:
        parsed.immediate_action = CliAction::kHelp;
        return parsed;
      case OptionType::kVersion:
        parsed.immediate_action = CliAction::kVersion;
        return parsed;
      case OptionType::kYear:
        parsed.single_year = ParseNextInt(args, i, "--year");
        break;
      case OptionType::kStart:
        parsed.start_year = ParseNextInt(args, i, "--start");
        break;
      case OptionType::kEnd:
        parsed.end_year = ParseNextInt(args, i, "--end");
        break;
      case OptionType::kItems:
        parsed.config.items_per_day = ParseNextInt(args, i, "--items");
        break;
      case OptionType::kNoSleep:
        parsed.config.enable_nosleep = true;
        break;
      case OptionType::kMonthlyAverage:
        parsed.config.enable_monthly_average_report = true;
        break;
      case OptionType::kOutput:
        parsed.config.output_directory = ParseNextString(args, i, "--output");
        break;
    }
  }

  return parsed;
}

void ApplyYearSelection(ParsedOptions& parsed) {
  if (parsed.single_year.has_value() &&
      (parsed.start_year.has_value() || parsed.end_year.has_value())) {
    throw std::logic_error("Cannot use --year together with --start or --end.");
  }

  if (parsed.single_year.has_value()) {
    parsed.config.mode = GenerationMode::SingleYear;
    parsed.config.start_year = *parsed.single_year;
    parsed.config.end_year = *parsed.single_year;
    return;
  }

  if (parsed.start_year.has_value() && parsed.end_year.has_value()) {
    parsed.config.mode = GenerationMode::YearRange;
    parsed.config.start_year = *parsed.start_year;
    parsed.config.end_year = *parsed.end_year;
    return;
  }

  throw std::logic_error(
      "You must specify either a single year with --year, or a range with "
      "--start and --end.");
}

void ValidateConfig(const Config& config) {
  if (config.start_year < kMinYear || config.end_year < kMinYear) {
    throw std::logic_error("Years must be positive integers.");
  }
  if (config.items_per_day < kMinItemsPerDay) {
    throw std::logic_error(
        "--items must be 2 or greater to generate realistic sleep data.");
  }
  if (config.end_year < config.start_year) {
    throw std::logic_error("--end year cannot be earlier than --start year.");
  }
  if (config.output_directory.empty()) {
    throw std::logic_error("--output directory cannot be empty.");
  }
}

}  // namespace

CommandLineParser::CommandLineParser(std::span<char* const> argv) {
  if (!argv.empty() && argv.front() != nullptr) {
    prog_name_ = argv.front();
  }
  for (size_t i = 1; i < argv.size(); ++i) {
    args_.emplace_back(argv[i]);
  }
}

auto CommandLineParser::parse() -> CliRequest {
  try {
    ParsedOptions parsed = ParseOptions(args_);
    if (parsed.immediate_action.has_value()) {
      return CliRequest{.action = *parsed.immediate_action,
                        .config = std::nullopt,
                        .error_message = {}};
    }

    ApplyYearSelection(parsed);
    ValidateConfig(parsed.config);

    return CliRequest{.action = CliAction::kRun,
                      .config = parsed.config,
                      .error_message = {}};
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
}

auto CommandLineParser::prog_name() const -> std::string_view {
  return prog_name_;
}
