#include "application/activity_name_converter.hpp"

#include <algorithm>
#include <cstddef>
#include <string>
#include <string_view>

#include "domain/types/converter_config.hpp"

import tracer.core.shared.canonical_text;

namespace {

constexpr std::string_view kRemarkDelimiter = "//";

[[nodiscard]] auto IsAsciiDigit(const char value) -> bool {
  return value >= '0' && value <= '9';
}

[[nodiscard]] auto IsAsciiSpace(const char value) -> bool {
  return value == ' ' || value == '\t';
}

[[nodiscard]] auto CountDigits(std::string_view line, const std::size_t start,
                               const std::size_t count) -> bool {
  return start + count <= line.size() &&
         std::ranges::all_of(line.substr(start, count), IsAsciiDigit);
}

[[nodiscard]] auto DetectClockLength(std::string_view line,
                                     const std::size_t start) -> std::size_t {
  if (CountDigits(line, start, 6)) {
    return 6;
  }
  if (CountDigits(line, start, 4)) {
    return 4;
  }
  return 0;
}

[[nodiscard]] auto FindRemarkStart(std::string_view line,
                                   const std::size_t start) -> std::size_t {
  return line.find(kRemarkDelimiter, start);
}

}  // namespace

ActivityNameTextConverter::ActivityNameTextConverter(
    const ConverterConfig& config) {
  for (const auto& wake_keyword : config.sleep_inference.wake_keywords) {
    if (!wake_keyword.empty()) {
      wake_keywords_.insert(wake_keyword);
    }
  }

  for (const auto& [alias, canonical] : config.text_mapping) {
    if (alias.empty() || canonical.empty()) {
      continue;
    }

    aliases_.insert(alias);
    canonical_names_.insert(canonical);

    const bool is_wake_keyword = wake_keywords_.contains(alias);
    if (is_wake_keyword) {
      wake_canonical_names_.insert(canonical);
      continue;
    }

    alias_to_canonical_.emplace(alias, canonical);

    const auto existing = canonical_to_alias_.find(canonical);
    if (existing == canonical_to_alias_.end() || alias < existing->second) {
      canonical_to_alias_[canonical] = alias;
    }
  }
}

auto ActivityNameTextConverter::ConvertName(
    const std::string_view name,
    const ActivityNameMappingDirection direction) const -> std::string {
  const std::string original(name);
  if (original.empty()) {
    return original;
  }

  if (direction == ActivityNameMappingDirection::kAliasToCanonical) {
    // Canonical names take precedence when a token is present in both sides of
    // the mapping. This is what makes canonical input genuinely idempotent.
    if (canonical_names_.contains(original) ||
        wake_keywords_.contains(original)) {
      return original;
    }
    const auto mapping = alias_to_canonical_.find(original);
    return mapping == alias_to_canonical_.end() ? original : mapping->second;
  }

  // An alias that is already present in the input remains untouched. This is
  // important for aliases such as a user-facing short name that also happens
  // to be a canonical value of another declaration.
  if (aliases_.contains(original) || wake_canonical_names_.contains(original)) {
    return original;
  }
  const auto mapping = canonical_to_alias_.find(original);
  return mapping == canonical_to_alias_.end() ? original : mapping->second;
}

auto ActivityNameTextConverter::ConvertText(
    const std::string_view text,
    const ActivityNameMappingDirection direction) const -> std::string {
  const std::string canonical_text =
      tracer::core::shared::canonical_text::RequireCanonicalText(
          text, "activity-name conversion input");

  std::string converted;
  converted.reserve(canonical_text.size());

  std::size_t line_start = 0;
  while (line_start < canonical_text.size()) {
    const std::size_t newline = canonical_text.find('\n', line_start);
    const std::size_t line_end =
        newline == std::string::npos ? canonical_text.size() : newline;
    converted +=
        ConvertEventLine(std::string_view(canonical_text)
                             .substr(line_start, line_end - line_start),
                         direction);
    if (newline == std::string::npos) {
      break;
    }
    converted.push_back('\n');
    line_start = newline + 1;
  }

  return converted;
}

auto ActivityNameTextConverter::ReplaceCanonicalNames(
    const std::string_view text,
    const std::unordered_map<std::string, std::string>& replacements) const
    -> std::string {
  const std::string canonical_text =
      tracer::core::shared::canonical_text::RequireCanonicalText(
          text, "canonical activity replacement input");

  std::string replaced;
  replaced.reserve(canonical_text.size());
  std::size_t line_start = 0;
  while (line_start < canonical_text.size()) {
    const std::size_t newline = canonical_text.find('\n', line_start);
    const std::size_t line_end =
        newline == std::string::npos ? canonical_text.size() : newline;
    replaced += ReplaceCanonicalNamesInEventLine(
        std::string_view(canonical_text).substr(line_start,
                                                 line_end - line_start),
        replacements);
    if (newline == std::string::npos) {
      break;
    }
    replaced.push_back('\n');
    line_start = newline + 1;
  }
  return replaced;
}

auto ActivityNameTextConverter::ConvertEventLine(
    const std::string_view line,
    const ActivityNameMappingDirection direction) const -> std::string {
  std::size_t event_start = 0;
  while (event_start < line.size() && IsAsciiSpace(line[event_start])) {
    ++event_start;
  }
  if (event_start == line.size()) {
    return std::string(line);
  }

  const std::size_t start_clock_length = DetectClockLength(line, event_start);
  if (start_clock_length == 0) {
    return std::string(line);
  }

  std::size_t activity_start = event_start + start_clock_length;
  if (activity_start < line.size() && line[activity_start] == '-') {
    const std::size_t end_clock_start = activity_start + 1;
    const std::size_t end_clock_length =
        DetectClockLength(line, end_clock_start);
    if (end_clock_length == 0) {
      return std::string(line);
    }
    activity_start = end_clock_start + end_clock_length;
  }

  if (activity_start >= line.size()) {
    return std::string(line);
  }

  const std::size_t remark_start = FindRemarkStart(line, activity_start);
  const std::size_t activity_limit =
      remark_start == std::string_view::npos ? line.size() : remark_start;

  std::size_t name_start = activity_start;
  while (name_start < activity_limit && IsAsciiSpace(line[name_start])) {
    ++name_start;
  }
  std::size_t name_end = activity_limit;
  while (name_end > name_start && IsAsciiSpace(line[name_end - 1])) {
    --name_end;
  }
  if (name_start == name_end) {
    return std::string(line);
  }

  const std::string converted_name =
      ConvertName(line.substr(name_start, name_end - name_start), direction);
  if (converted_name == line.substr(name_start, name_end - name_start)) {
    return std::string(line);
  }

  std::string converted;
  converted.reserve(line.size() - (name_end - name_start) +
                    converted_name.size());
  converted.append(line.substr(0, name_start));
  converted.append(converted_name);
  converted.append(line.substr(name_end));
  return converted;
}

auto ActivityNameTextConverter::ReplaceCanonicalNamesInEventLine(
    const std::string_view line,
    const std::unordered_map<std::string, std::string>& replacements) const
    -> std::string {
  std::size_t event_start = 0;
  while (event_start < line.size() && IsAsciiSpace(line[event_start])) {
    ++event_start;
  }
  if (event_start == line.size()) {
    return std::string(line);
  }
  const std::size_t start_clock_length = DetectClockLength(line, event_start);
  if (start_clock_length == 0) {
    return std::string(line);
  }
  std::size_t activity_start = event_start + start_clock_length;
  if (activity_start < line.size() && line[activity_start] == '-') {
    const std::size_t end_clock_start = activity_start + 1;
    const std::size_t end_clock_length =
        DetectClockLength(line, end_clock_start);
    if (end_clock_length == 0) {
      return std::string(line);
    }
    activity_start = end_clock_start + end_clock_length;
  }
  const std::size_t remark_start = FindRemarkStart(line, activity_start);
  const std::size_t activity_limit =
      remark_start == std::string_view::npos ? line.size() : remark_start;
  std::size_t name_start = activity_start;
  while (name_start < activity_limit && IsAsciiSpace(line[name_start])) {
    ++name_start;
  }
  std::size_t name_end = activity_limit;
  while (name_end > name_start && IsAsciiSpace(line[name_end - 1])) {
    --name_end;
  }
  if (name_start == name_end) {
    return std::string(line);
  }
  const std::string name(line.substr(name_start, name_end - name_start));
  const auto replacement = replacements.find(name);
  if (replacement == replacements.end()) {
    return std::string(line);
  }
  std::string replaced;
  replaced.reserve(line.size() - name.size() + replacement->second.size());
  replaced.append(line.substr(0, name_start));
  replaced.append(replacement->second);
  replaced.append(line.substr(name_end));
  return replaced;
}
