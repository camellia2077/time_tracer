// cli/framework/command_line_parser.hpp
#ifndef CLI_FRAMEWORK_COMMAND_LINE_PARSER_H_
#define CLI_FRAMEWORK_COMMAND_LINE_PARSER_H_

#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "cli/framework/cli_request.hpp"

class CommandLineParser {
 public:
  explicit CommandLineParser(std::span<char* const> argv);
  CliRequest parse();
  std::string_view prog_name() const;

 private:
  std::string prog_name_;
  std::vector<std::string> args_;
};

#endif  // CLI_FRAMEWORK_COMMAND_LINE_PARSER_H_
