// shared/types/ansi_colors.hpp
#ifndef COMMON_ANSI_COLORS_H_
#define COMMON_ANSI_COLORS_H_

#include <string_view>

namespace time_tracer::common::colors {

// --- ANSI Color Code Definitions ---
constexpr std::string_view kReset = "\033[0m";

// Styles
constexpr std::string_view kBold = "\033[1m";
constexpr std::string_view kItalic = "\033[3m";

// Standard Colors
constexpr std::string_view kRed = "\033[31m";
constexpr std::string_view kGreen = "\033[32m";
constexpr std::string_view kYellow = "\033[33m";
constexpr std::string_view kCyan = "\033[36m";
constexpr std::string_view kGray = "\033[90m";

// Bright Colors
constexpr std::string_view kBrightGreen = "\033[92m";
constexpr std::string_view kBrightCyan = "\033[96m";

}  // namespace time_tracer::common::colors

#endif  // COMMON_ANSI_COLORS_H_