// common/ansi_colors.hpp
#ifndef COMMON_ANSI_COLORS_H_
#define COMMON_ANSI_COLORS_H_

#include <string_view>

namespace time_tracer::common::colors {

// --- ANSI Color Code Definitions ---
constexpr std::string_view kReset = "\033[0m";
constexpr std::string_view kRed = "\033[31m";
constexpr std::string_view kGreen = "\033[32m";
constexpr std::string_view kYellow = "\033[33m";
constexpr std::string_view kBlue = "\033[34m";
constexpr std::string_view kMagenta = "\033[35m";
constexpr std::string_view kCyan = "\033[36m";

}  // namespace time_tracer::common::colors

#endif  // COMMON_ANSI_COLORS_H_