module;

#include "shared/types/ansi_colors.hpp"

export module tracer.core.shared.ansi_colors;

export namespace tracer::core::shared::modcolors {

inline constexpr std::string_view kReset = "\033[0m";
inline constexpr std::string_view kBold = "\033[1m";
inline constexpr std::string_view kItalic = "\033[3m";
inline constexpr std::string_view kRed = "\033[31m";
inline constexpr std::string_view kGreen = "\033[32m";
inline constexpr std::string_view kYellow = "\033[33m";
inline constexpr std::string_view kCyan = "\033[36m";
inline constexpr std::string_view kGray = "\033[90m";
inline constexpr std::string_view kBrightGreen = "\033[92m";
inline constexpr std::string_view kBrightCyan = "\033[96m";

}  // namespace tracer::core::shared::modcolors
