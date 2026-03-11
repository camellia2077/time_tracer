// shared/types/ansi_colors.hpp
#ifndef SHARED_TYPES_ANSI_COLORS_H_
#define SHARED_TYPES_ANSI_COLORS_H_

#include <string_view>

namespace tracer_core::common::colors {

// --- ANSI Color Code Definitions ---
inline constexpr std::string_view kReset = "\033[0m";

// Styles
inline constexpr std::string_view kBold = "\033[1m";
inline constexpr std::string_view kItalic = "\033[3m";

// Standard Colors
inline constexpr std::string_view kRed = "\033[31m";
inline constexpr std::string_view kGreen = "\033[32m";
inline constexpr std::string_view kYellow = "\033[33m";
inline constexpr std::string_view kCyan = "\033[36m";
inline constexpr std::string_view kGray = "\033[90m";

// Bright Colors
inline constexpr std::string_view kBrightGreen = "\033[92m";
inline constexpr std::string_view kBrightCyan = "\033[96m";

}  // namespace tracer_core::common::colors

namespace tracer::core::shared::ansi_colors {

#include "shared/detail/ansi_colors_contract.inc"

}  // namespace tracer::core::shared::ansi_colors

#endif  // SHARED_TYPES_ANSI_COLORS_H_
