#include "application/pipeline/txt_day_block_support.hpp"

#include <algorithm>
#include <cctype>
#include <format>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "shared/utils/canonical_text.hpp"
#include "shared/utils/string_utils.hpp"

namespace tracer::core::application::pipeline::txt_day_block {

namespace {

namespace modtext = tracer::core::shared::canonical_text;
using tracer::core::shared::string_utils::Trim;
using tracer_core::core::dto::DefaultTxtDayMarkerRequest;
using tracer_core::core::dto::DefaultTxtDayMarkerResponse;
using tracer_core::core::dto::ReplaceTxtDayBlockRequest;
using tracer_core::core::dto::ReplaceTxtDayBlockResponse;
using tracer_core::core::dto::ResolveTxtDayBlockRequest;
using tracer_core::core::dto::ResolveTxtDayBlockResponse;

struct ParsedIsoDate {
  int year = 0;
  int month = 0;
  int day = 0;
  std::string day_marker;
};

struct ParsedYearMonth {
  int year = 0;
  int month = 0;
};

[[nodiscard]] auto IsDigitsOnly(std::string_view value) -> bool {
  return !value.empty() &&
         std::ranges::all_of(value, [](const char ch) -> bool {
           return std::isdigit(static_cast<unsigned char>(ch)) != 0;
         });
}

[[nodiscard]] auto IsLeapYear(const int year) -> bool {
  return (year % 400 == 0) || ((year % 4 == 0) && (year % 100 != 0));
}

[[nodiscard]] auto DaysInMonth(const int year, const int month) -> int {
  switch (month) {
    case 1:
    case 3:
    case 5:
    case 7:
    case 8:
    case 10:
    case 12:
      return 31;
    case 4:
    case 6:
    case 9:
    case 11:
      return 30;
    case 2:
      return IsLeapYear(year) ? 29 : 28;
    default:
      return 0;
  }
}

[[nodiscard]] auto ParseIsoDate(std::string_view raw_target_date)
    -> ParsedIsoDate {
  const std::string trimmed = Trim(std::string(raw_target_date));
  if (trimmed.size() != 10U || trimmed[4] != '-' || trimmed[7] != '-' ||
      !IsDigitsOnly(trimmed.substr(0, 4)) ||
      !IsDigitsOnly(trimmed.substr(5, 2)) ||
      !IsDigitsOnly(trimmed.substr(8, 2))) {
    throw std::invalid_argument("target_date_iso must use YYYY-MM-DD.");
  }

  const int year = std::stoi(trimmed.substr(0, 4));
  const int month = std::stoi(trimmed.substr(5, 2));
  const int day = std::stoi(trimmed.substr(8, 2));
  const int max_day = DaysInMonth(year, month);
  if (month < 1 || month > 12 || day < 1 || day > max_day) {
    throw std::invalid_argument("target_date_iso is not a valid calendar date.");
  }

  return ParsedIsoDate{
      .year = year,
      .month = month,
      .day = day,
      .day_marker = std::format("{:02d}{:02d}", month, day),
  };
}

[[nodiscard]] auto TryParseSelectedMonth(std::string_view raw_value)
    -> std::optional<ParsedYearMonth> {
  const std::string trimmed = Trim(std::string(raw_value));
  if (trimmed.size() != 7U || trimmed[4] != '-' ||
      !IsDigitsOnly(trimmed.substr(0, 4)) ||
      !IsDigitsOnly(trimmed.substr(5, 2))) {
    return std::nullopt;
  }

  const int year = std::stoi(trimmed.substr(0, 4));
  const int month = std::stoi(trimmed.substr(5, 2));
  if (month < 1 || month > 12) {
    return std::nullopt;
  }
  return ParsedYearMonth{.year = year, .month = month};
}

[[nodiscard]] auto NormalizeDayMarker(std::string_view raw_value)
    -> std::string {
  std::string normalized;
  normalized.reserve(4);
  for (const char ch : raw_value) {
    if (std::isdigit(static_cast<unsigned char>(ch)) == 0) {
      continue;
    }
    if (normalized.size() >= 4U) {
      break;
    }
    normalized.push_back(ch);
  }
  return normalized;
}

[[nodiscard]] auto IsValidDayMarker(std::string_view value) -> bool {
  if (value.size() != 4U || !IsDigitsOnly(value)) {
    return false;
  }
  const int month = std::stoi(std::string(value.substr(0, 2)));
  const int day = std::stoi(std::string(value.substr(2, 2)));
  return month >= 1 && month <= 12 && day >= 1 && day <= 31;
}

[[nodiscard]] auto IsDayMarkerLine(std::string_view line) -> bool {
  return IsValidDayMarker(Trim(std::string(line)));
}

[[nodiscard]] auto SplitLines(std::string_view content)
    -> std::vector<std::string> {
  std::vector<std::string> lines;
  std::string current;
  current.reserve(content.size());
  for (const char ch : content) {
    if (ch == '\n') {
      if (!current.empty() && current.back() == '\r') {
        current.pop_back();
      }
      lines.push_back(current);
      current.clear();
      continue;
    }
    current.push_back(ch);
  }
  if (!current.empty() && current.back() == '\r') {
    current.pop_back();
  }
  if (!current.empty() || (!content.empty() && content.back() != '\n')) {
    lines.push_back(current);
  }
  return lines;
}

[[nodiscard]] auto SplitLinesPreserveTrailingEmpty(std::string_view content)
    -> std::vector<std::string> {
  std::vector<std::string> lines = SplitLines(content);
  if (!content.empty() && content.back() == '\n') {
    lines.push_back("");
  }
  return lines;
}

[[nodiscard]] auto FindDayBlockStart(const std::vector<std::string>& lines,
                                     std::string_view day_marker) -> int {
  for (int index = 0; index < static_cast<int>(lines.size()); ++index) {
    if (Trim(lines[static_cast<std::size_t>(index)]) == day_marker) {
      return index;
    }
  }
  return -1;
}

[[nodiscard]] auto FindDayBlockEnd(const std::vector<std::string>& lines,
                                   const int block_start) -> int {
  for (int index = block_start + 1; index < static_cast<int>(lines.size());
       ++index) {
    if (IsDayMarkerLine(lines[static_cast<std::size_t>(index)])) {
      return index;
    }
  }
  return static_cast<int>(lines.size());
}

[[nodiscard]] auto NormalizeEditedDayBody(std::string_view normalized_day_marker,
                                          std::string_view edited_day_body)
    -> std::vector<std::string> {
  std::vector<std::string> lines =
      SplitLinesPreserveTrailingEmpty(edited_day_body);
  if (!lines.empty() && Trim(lines.front()) == normalized_day_marker) {
    lines.erase(lines.begin());
  }
  return lines;
}

[[nodiscard]] auto JoinLinesCanonical(const std::vector<std::string>& lines)
    -> std::string {
  std::string joined;
  for (const auto& line : lines) {
    joined.append(line);
    joined.push_back('\n');
  }
  return modtext::RequireCanonicalText(joined, "txt_day_block");
}

[[nodiscard]] auto BuildDayContentIsoDate(
    std::string_view selected_month, std::string_view normalized_day_marker)
    -> std::optional<std::string> {
  if (!IsValidDayMarker(normalized_day_marker)) {
    return std::nullopt;
  }
  const auto parsed_month = TryParseSelectedMonth(selected_month);
  if (!parsed_month.has_value()) {
    return std::nullopt;
  }

  const int marker_month =
      std::stoi(std::string(normalized_day_marker.substr(0, 2)));
  const int marker_day =
      std::stoi(std::string(normalized_day_marker.substr(2, 2)));
  if (marker_month != parsed_month->month ||
      marker_day > DaysInMonth(parsed_month->year, parsed_month->month)) {
    return std::nullopt;
  }
  return std::format("{:04d}-{:02d}-{:02d}", parsed_month->year,
                     marker_month, marker_day);
}

}  // namespace

auto DefaultDayMarker(const DefaultTxtDayMarkerRequest& request)
    -> DefaultTxtDayMarkerResponse {
  const ParsedIsoDate target_date = ParseIsoDate(request.target_date_iso);
  const auto parsed_month = TryParseSelectedMonth(request.selected_month);
  if (!parsed_month.has_value()) {
    return {.ok = true,
            .normalized_day_marker = target_date.day_marker,
            .error_message = ""};
  }

  const int max_day = DaysInMonth(parsed_month->year, parsed_month->month);
  return {
      .ok = true,
      .normalized_day_marker = std::format(
          "{:02d}{:02d}", parsed_month->month,
          std::min(target_date.day, max_day > 0 ? max_day : target_date.day)),
      .error_message = "",
  };
}

auto ResolveDayBlock(const ResolveTxtDayBlockRequest& request)
    -> ResolveTxtDayBlockResponse {
  const std::string normalized_day_marker = NormalizeDayMarker(request.day_marker);
  const std::optional<std::string> iso_date =
      BuildDayContentIsoDate(request.selected_month, normalized_day_marker);
  if (!IsValidDayMarker(normalized_day_marker)) {
    return {.ok = true,
            .normalized_day_marker = normalized_day_marker,
            .found = false,
            .is_marker_valid = false,
            .can_save = false,
            .day_body = "",
            .day_content_iso_date = iso_date,
            .error_message = ""};
  }

  const std::vector<std::string> lines = SplitLines(request.content);
  const int start_index = FindDayBlockStart(lines, normalized_day_marker);
  if (start_index < 0) {
    return {.ok = true,
            .normalized_day_marker = normalized_day_marker,
            .found = false,
            .is_marker_valid = true,
            .can_save = false,
            .day_body = "",
            .day_content_iso_date = iso_date,
            .error_message = ""};
  }

  const int end_index = FindDayBlockEnd(lines, start_index);
  std::string day_body;
  for (int index = start_index + 1; index < end_index; ++index) {
    if (!day_body.empty()) {
      day_body.push_back('\n');
    }
    day_body.append(lines[static_cast<std::size_t>(index)]);
  }

  return {.ok = true,
          .normalized_day_marker = normalized_day_marker,
          .found = true,
          .is_marker_valid = true,
          .can_save = true,
          .day_body = day_body,
          .day_content_iso_date = iso_date,
          .error_message = ""};
}

auto ReplaceDayBlock(const ReplaceTxtDayBlockRequest& request)
    -> ReplaceTxtDayBlockResponse {
  const std::string normalized_day_marker = NormalizeDayMarker(request.day_marker);
  if (!IsValidDayMarker(normalized_day_marker)) {
    return {.ok = true,
            .normalized_day_marker = normalized_day_marker,
            .found = false,
            .is_marker_valid = false,
            .updated_content = request.content,
            .error_message = ""};
  }

  std::vector<std::string> lines = SplitLines(request.content);
  const int start_index = FindDayBlockStart(lines, normalized_day_marker);
  if (start_index < 0) {
    return {.ok = true,
            .normalized_day_marker = normalized_day_marker,
            .found = false,
            .is_marker_valid = true,
            .updated_content = request.content,
            .error_message = ""};
  }

  const int end_index = FindDayBlockEnd(lines, start_index);
  const std::vector<std::string> normalized_body_lines =
      NormalizeEditedDayBody(normalized_day_marker, request.edited_day_body);
  lines.erase(lines.begin() + start_index + 1, lines.begin() + end_index);
  lines.insert(lines.begin() + start_index + 1, normalized_body_lines.begin(),
               normalized_body_lines.end());

  return {.ok = true,
          .normalized_day_marker = normalized_day_marker,
          .found = true,
          .is_marker_valid = true,
          .updated_content = JoinLinesCanonical(lines),
          .error_message = ""};
}

}  // namespace tracer::core::application::pipeline::txt_day_block
