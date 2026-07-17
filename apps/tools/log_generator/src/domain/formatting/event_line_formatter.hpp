#ifndef DOMAIN_FORMATTING_EVENT_LINE_FORMATTER_HPP_
#define DOMAIN_FORMATTING_EVENT_LINE_FORMATTER_HPP_

#include <optional>
#include <string>
#include <string_view>

#include "common/config_types.hpp"
#include "domain/model/generated_event.hpp"

namespace EventLineFormatter {

// Formatter owns TXT line rendering only. Timeline generation stays in
// EventGenerator and related scheduling components.
auto format_point_event_line(int end_minute, std::string_view activity_token,
                             const std::optional<std::string>& remark_suffix,
                             TimeFormat time_format = TimeFormat::Hhmmss)
    -> std::string;

auto format_interval_event_line(
    int start_minute, int end_minute, std::string_view activity_token,
    const std::optional<std::string>& remark_suffix,
    TimeFormat time_format = TimeFormat::Hhmmss) -> std::string;

void append_formatted_event(std::string& buffer, const GeneratedEvent& event,
                            TimeFormat time_format = TimeFormat::Hhmmss);

}  // namespace EventLineFormatter

#endif  // DOMAIN_FORMATTING_EVENT_LINE_FORMATTER_HPP_
