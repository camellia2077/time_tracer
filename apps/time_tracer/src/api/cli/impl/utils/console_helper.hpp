// api/cli/impl/utils/console_helper.hpp
#pragma once

#include <cstdio>
#include <format>
#include <iostream>
#include <print>
#include <string>
#include <vector>

namespace ConsoleHelper {

#if defined(_WIN32) || defined(_WIN64)
// Setup Windows console (UTF-8, VT processing)
void SetupConsole();

// Get command line arguments in UTF-8
auto GetUtf8Args() -> std::vector<std::string>;
#endif

// Safe println helper (std::ostream)
template <typename... Args>
void SafePrintln(std::ostream& stream, std::format_string<Args...> fmt,
                 Args&&... args) noexcept {
  try {
    std::println(stream, fmt, std::forward<Args>(args)...);
  } catch (...) {  // NOLINT(bugprone-empty-catch)
    // Intentionally ignore output failures
  }
}

// Safe println helper (std::FILE*)
template <typename... Args>
void SafePrintln(std::FILE* stream, std::format_string<Args...> fmt,
                 Args&&... args) noexcept {
  try {
    std::println(stream, fmt, std::forward<Args>(args)...);
  } catch (...) {  // NOLINT(bugprone-empty-catch)
    // Intentionally ignore output failures
  }
}

// Safe println helper (stdout default)
template <typename... Args>
void SafePrintln(std::format_string<Args...> fmt, Args&&... args) noexcept {
  try {
    std::println(fmt, std::forward<Args>(args)...);
  } catch (...) {  // NOLINT(bugprone-empty-catch)
    // Intentionally ignore output failures
  }
}

}  // namespace ConsoleHelper
