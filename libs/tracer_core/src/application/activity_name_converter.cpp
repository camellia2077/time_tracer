#include "application/activity_name_converter.hpp"

#include <algorithm>
#include <cstddef>
#include <string>
#include <string_view>

#include "domain/types/converter_config.hpp"

import tracer.core.shared.canonical_text;

namespace {

constexpr std::string_view kRemarkDelimiter = "//";

[[nodiscard]] auto IsAsciiDigit(const char kValue) -> bool {
  return kValue >= '0' && kValue <= '9';
}

[[nodiscard]] auto IsAsciiSpace(const char kValue) -> bool {
  return kValue == ' ' || kValue == '\t';
}

[[nodiscard]] auto CountDigits(std::string_view line, const std::size_t kStart,
                               const std::size_t kCount) -> bool {
  return kStart + kCount <= line.size() &&
         std::ranges::all_of(line.substr(kStart, kCount), IsAsciiDigit);
}

[[nodiscard]] auto DetectClockLength(std::string_view line,
                                     const std::size_t kStart) -> std::size_t {
  if (CountDigits(line, kStart, 6)) {
    return 6;
  }
  if (CountDigits(line, kStart, 4)) {
    return 4;
  }
  return 0;
}

[[nodiscard]] auto FindRemarkStart(std::string_view line,
                                   const std::size_t kStart) -> std::size_t {
  return line.find(kRemarkDelimiter, kStart);
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

    const bool kIsWakeKeyword = wake_keywords_.contains(alias);
    if (kIsWakeKeyword) {
      wake_canonical_names_.insert(canonical);
      continue;
    }

    alias_to_canonical_.emplace(alias, canonical);

    const auto kExisting = canonical_to_alias_.find(canonical);
    if (kExisting == canonical_to_alias_.end() || alias < kExisting->second) {
      canonical_to_alias_[canonical] = alias;
    }
  }
}

auto ActivityNameTextConverter::ConvertName(
    const std::string_view kName,
    const ActivityNameMappingDirection kDirection) const -> std::string {
  const std::string kOriginal(kName);
  if (kOriginal.empty()) {
    return kOriginal;
  }

  if (kDirection == ActivityNameMappingDirection::kAliasToCanonical) {
    // Canonical names take precedence when a token is present in both sides of
    // the mapping. This is what makes canonical input genuinely idempotent.
    if (canonical_names_.contains(kOriginal) ||
        wake_keywords_.contains(kOriginal)) {
      return kOriginal;
    }
    const auto kMapping = alias_to_canonical_.find(kOriginal);
    return kMapping == alias_to_canonical_.end() ? kOriginal : kMapping->second;
  }

  // An alias that is already present in the input remains untouched. This is
  // important for aliases such as a user-facing short name that also happens
  // to be a canonical value of another declaration.
  if (aliases_.contains(kOriginal) ||
      wake_canonical_names_.contains(kOriginal)) {
    return kOriginal;
  }
  const auto kMapping = canonical_to_alias_.find(kOriginal);
  return kMapping == canonical_to_alias_.end() ? kOriginal : kMapping->second;
}

auto ActivityNameTextConverter::ConvertText(
    const std::string_view kText,
    const ActivityNameMappingDirection kDirection) const -> std::string {
  const std::string kCanonicalText =
      tracer::core::shared::canonical_text::RequireCanonicalText(
          kText, "activity-name conversion input");

  std::string converted;
  converted.reserve(kCanonicalText.size());

  std::size_t line_start = 0;
  while (line_start < kCanonicalText.size()) {
    const std::size_t kNewline = kCanonicalText.find('\n', line_start);
    const std::size_t kLineEnd =
        kNewline == std::string::npos ? kCanonicalText.size() : kNewline;
    converted +=
        ConvertEventLine(std::string_view(kCanonicalText)
                             .substr(line_start, kLineEnd - line_start),
                         kDirection);
    if (kNewline == std::string::npos) {
      break;
    }
    converted.push_back('\n');
    line_start = kNewline + 1;
  }

  return converted;
}

auto ActivityNameTextConverter::ReplaceCanonicalNames(
    const std::string_view kText,
    const std::unordered_map<std::string, std::string>& replacements) const
    -> std::string {
  const std::string kCanonicalText =
      tracer::core::shared::canonical_text::RequireCanonicalText(
          kText, "canonical activity replacement input");

  std::string replaced;
  replaced.reserve(kCanonicalText.size());
  std::size_t line_start = 0;
  while (line_start < kCanonicalText.size()) {
    const std::size_t kNewline = kCanonicalText.find('\n', line_start);
    const std::size_t kLineEnd =
        kNewline == std::string::npos ? kCanonicalText.size() : kNewline;
    replaced += ReplaceCanonicalNamesInEventLine(
        std::string_view(kCanonicalText)
            .substr(line_start, kLineEnd - line_start),
        replacements);
    if (kNewline == std::string::npos) {
      break;
    }
    replaced.push_back('\n');
    line_start = kNewline + 1;
  }
  return replaced;
}

auto ActivityNameTextConverter::ReplaceAliasNames(
    const std::string_view kText,
    const std::unordered_map<std::string, std::string>& replacements) const
    -> std::string {
  const std::string kCanonicalText =
      tracer::core::shared::canonical_text::RequireCanonicalText(
          kText, "alias activity replacement input");

  std::string replaced;
  replaced.reserve(kCanonicalText.size());
  std::size_t line_start = 0;
  while (line_start < kCanonicalText.size()) {
    const std::size_t kNewline = kCanonicalText.find('\n', line_start);
    const std::size_t kLineEnd =
        kNewline == std::string::npos ? kCanonicalText.size() : kNewline;
    replaced += ReplaceCanonicalNamesInEventLine(
        std::string_view(kCanonicalText)
            .substr(line_start, kLineEnd - line_start),
        replacements);
    if (kNewline == std::string::npos) {
      break;
    }
    replaced.push_back('\n');
    line_start = kNewline + 1;
  }
  return replaced;
}

auto ActivityNameTextConverter::ConvertEventLine(
    const std::string_view kLine,
    const ActivityNameMappingDirection kDirection) const -> std::string {
  std::size_t event_start = 0;
  while (event_start < kLine.size() && IsAsciiSpace(kLine[event_start])) {
    ++event_start;
  }
  if (event_start == kLine.size()) {
    return std::string(kLine);
  }

  const std::size_t kStartClockLength = DetectClockLength(kLine, event_start);
  if (kStartClockLength == 0) {
    return std::string(kLine);
  }

  std::size_t activity_start = event_start + kStartClockLength;
  if (activity_start < kLine.size() && kLine[activity_start] == '-') {
    const std::size_t kEndClockStart = activity_start + 1;
    const std::size_t kEndClockLength =
        DetectClockLength(kLine, kEndClockStart);
    if (kEndClockLength == 0) {
      return std::string(kLine);
    }
    activity_start = kEndClockStart + kEndClockLength;
  }

  if (activity_start >= kLine.size()) {
    return std::string(kLine);
  }

  const std::size_t kRemarkStart = FindRemarkStart(kLine, activity_start);
  const std::size_t kActivityLimit =
      kRemarkStart == std::string_view::npos ? kLine.size() : kRemarkStart;

  std::size_t name_start = activity_start;
  while (name_start < kActivityLimit && IsAsciiSpace(kLine[name_start])) {
    ++name_start;
  }
  std::size_t name_end = kActivityLimit;
  while (name_end > name_start && IsAsciiSpace(kLine[name_end - 1])) {
    --name_end;
  }
  if (name_start == name_end) {
    return std::string(kLine);
  }

  const std::string kConvertedName =
      ConvertName(kLine.substr(name_start, name_end - name_start), kDirection);
  if (kConvertedName == kLine.substr(name_start, name_end - name_start)) {
    return std::string(kLine);
  }

  std::string converted;
  converted.reserve(kLine.size() - (name_end - name_start) +
                    kConvertedName.size());
  converted.append(kLine.substr(0, name_start));
  converted.append(kConvertedName);
  converted.append(kLine.substr(name_end));
  return converted;
}

auto ActivityNameTextConverter::ReplaceCanonicalNamesInEventLine(
    const std::string_view kLine,
    const std::unordered_map<std::string, std::string>& replacements) const
    -> std::string {
  std::size_t event_start = 0;
  while (event_start < kLine.size() && IsAsciiSpace(kLine[event_start])) {
    ++event_start;
  }
  if (event_start == kLine.size()) {
    return std::string(kLine);
  }
  const std::size_t kStartClockLength = DetectClockLength(kLine, event_start);
  if (kStartClockLength == 0) {
    return std::string(kLine);
  }
  std::size_t activity_start = event_start + kStartClockLength;
  if (activity_start < kLine.size() && kLine[activity_start] == '-') {
    const std::size_t kEndClockStart = activity_start + 1;
    const std::size_t kEndClockLength =
        DetectClockLength(kLine, kEndClockStart);
    if (kEndClockLength == 0) {
      return std::string(kLine);
    }
    activity_start = kEndClockStart + kEndClockLength;
  }
  const std::size_t kRemarkStart = FindRemarkStart(kLine, activity_start);
  const std::size_t kActivityLimit =
      kRemarkStart == std::string_view::npos ? kLine.size() : kRemarkStart;
  std::size_t name_start = activity_start;
  while (name_start < kActivityLimit && IsAsciiSpace(kLine[name_start])) {
    ++name_start;
  }
  std::size_t name_end = kActivityLimit;
  while (name_end > name_start && IsAsciiSpace(kLine[name_end - 1])) {
    --name_end;
  }
  if (name_start == name_end) {
    return std::string(kLine);
  }
  const std::string kName(kLine.substr(name_start, name_end - name_start));
  const auto kReplacement = replacements.find(kName);
  if (kReplacement == replacements.end()) {
    return std::string(kLine);
  }
  std::string replaced;
  replaced.reserve(kLine.size() - kName.size() + kReplacement->second.size());
  replaced.append(kLine.substr(0, name_start));
  replaced.append(kReplacement->second);
  replaced.append(kLine.substr(name_end));
  return replaced;
}
