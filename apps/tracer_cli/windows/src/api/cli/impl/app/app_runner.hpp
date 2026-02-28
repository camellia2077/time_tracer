// api/cli/impl/app/app_runner.hpp
#pragma once

#include <string>
#include <vector>

namespace AppRunner {

// Main entry point for the application logic
// Returns the exit code
auto Run(std::vector<std::string> args) -> int;

} // namespace AppRunner
