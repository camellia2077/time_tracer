module;

#include "shared/types/ansi_colors.hpp"

export module tracer.core.shared.ansi_colors;

export namespace tracer::core::shared::ansi_colors {

#include "shared/detail/ansi_colors_contract.inc"

}  // namespace tracer::core::shared::ansi_colors

export namespace tracer::core::shared::modcolors {

using tracer::core::shared::ansi_colors::kBold;
using tracer::core::shared::ansi_colors::kBrightCyan;
using tracer::core::shared::ansi_colors::kBrightGreen;
using tracer::core::shared::ansi_colors::kCyan;
using tracer::core::shared::ansi_colors::kGray;
using tracer::core::shared::ansi_colors::kGreen;
using tracer::core::shared::ansi_colors::kItalic;
using tracer::core::shared::ansi_colors::kRed;
using tracer::core::shared::ansi_colors::kReset;
using tracer::core::shared::ansi_colors::kYellow;

}  // namespace tracer::core::shared::modcolors
