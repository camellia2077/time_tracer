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
         std::ranges::all_of(value, [](const char kCh) -> bool {
           return std::isdigit(static_cast<unsigned char>(kCh)) != 0;
         });
}

[[nodiscard]] auto IsLeapYear(const int kYear) -> bool {
  return (kYear % 400 == 0) || ((kYear % 4 == 0) && (kYear % 100 != 0));
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
[[nodiscard]] auto DaysInMonth(const int kYear, const int kMonth) -> int {
  switch (kMonth) {
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
      return IsLeapYear(kYear) ? 29 : 28;
    default:
      return 0;
  }
}

[[nodiscard]] auto ParseIsoDate(std::string_view raw_target_date)
    -> ParsedIsoDate {
  const std::string kTrimmed = Trim(std::string(raw_target_date));
  if (kTrimmed.size() != 10U || kTrimmed[4] != '-' || kTrimmed[7] != '-' ||
      !IsDigitsOnly(kTrimmed.substr(0, 4)) ||
      !IsDigitsOnly(kTrimmed.substr(5, 2)) ||
      !IsDigitsOnly(kTrimmed.substr(8, 2))) {
    throw std::invalid_argument("target_date_iso must use YYYY-MM-DD.");
  }

  const int kYear = std::stoi(kTrimmed.substr(0, 4));
  const int kMonth = std::stoi(kTrimmed.substr(5, 2));
  const int kDay = std::stoi(kTrimmed.substr(8, 2));
  const int kMaxDay = DaysInMonth(kYear, kMonth);
  if (kMonth < 1 || kMonth > 12 || kDay < 1 || kDay > kMaxDay) {
    throw std::invalid_argument(
        "target_date_iso is not a valid calendar date.");
  }

  return ParsedIsoDate{
      .year = kYear,
      .month = kMonth,
      .day = kDay,
      .day_marker = std::format("{:02d}{:02d}", kMonth, kDay),
  };
}

[[nodiscard]] auto TryParseSelectedMonth(std::string_view raw_value)
    -> std::optional<ParsedYearMonth> {
  const std::string kTrimmed = Trim(std::string(raw_value));
  if (kTrimmed.size() != 7U || kTrimmed[4] != '-' ||
      !IsDigitsOnly(kTrimmed.substr(0, 4)) ||
      !IsDigitsOnly(kTrimmed.substr(5, 2))) {
    return std::nullopt;
  }

  const int kYear = std::stoi(kTrimmed.substr(0, 4));
  const int kMonth = std::stoi(kTrimmed.substr(5, 2));
  if (kMonth < 1 || kMonth > 12) {
    return std::nullopt;
  }
  return ParsedYearMonth{.year = kYear, .month = kMonth};
}

[[nodiscard]] auto NormalizeDayMarker(std::string_view raw_value)
    -> std::string {
  std::string normalized;
  normalized.reserve(4);
  for (const char kCh : raw_value) {
    if (std::isdigit(static_cast<unsigned char>(kCh)) == 0) {
      continue;
    }
    if (normalized.size() >= 4U) {
      break;
    }
    normalized.push_back(kCh);
  }
  return normalized;
}

[[nodiscard]] auto IsValidDayMarker(std::string_view value) -> bool {
  if (value.size() != 4U || !IsDigitsOnly(value)) {
    return false;
  }
  const int kMonth = std::stoi(std::string(value.substr(0, 2)));
  const int kDay = std::stoi(std::string(value.substr(2, 2)));
  return kMonth >= 1 && kMonth <= 12 && kDay >= 1 && kDay <= 31;
}

[[nodiscard]] auto IsDayMarkerLine(std::string_view line) -> bool {
  const std::string kTrimmed = Trim(std::string(line));
  return kTrimmed.size() == 5U && kTrimmed.front() == 'd' &&
         IsValidDayMarker(std::string_view(kTrimmed).substr(1));
}

[[nodiscard]] auto BuildDayMarkerLine(std::string_view normalized_day_marker)
    -> std::string {
  return "d" + std::string(normalized_day_marker);
}

[[nodiscard]] auto SplitLines(std::string_view content)
    -> std::vector<std::string> {
  std::vector<std::string> lines;
  std::string current;
  current.reserve(content.size());
  for (const char kCh : content) {
    if (kCh == '\n') {
      if (!current.empty() && current.back() == '\r') {
        current.pop_back();
      }
      lines.push_back(current);
      current.clear();
      continue;
    }
    current.push_back(kCh);
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
  const std::string kDayMarkerLine = BuildDayMarkerLine(day_marker);
  for (int index = 0; index < static_cast<int>(lines.size()); ++index) {
    if (Trim(lines[static_cast<std::size_t>(index)]) == kDayMarkerLine) {
      return index;
    }
  }
  return -1;
}

[[nodiscard]] auto FindDayBlockEnd(const std::vector<std::string>& lines,
                                   const int kBlockStart) -> int {
  for (int index = kBlockStart + 1; index < static_cast<int>(lines.size());
       ++index) {
    if (IsDayMarkerLine(lines[static_cast<std::size_t>(index)])) {
      return index;
    }
  }
  return static_cast<int>(lines.size());
}

// NOLINTBEGIN(bugprone-easily-swappable-parameters)
[[nodiscard]] auto NormalizeEditedDayBody(
    std::string_view normalized_day_marker, std::string_view edited_day_body)
    -> std::vector<std::string> {
  std::vector<std::string> lines =
      SplitLinesPreserveTrailingEmpty(edited_day_body);
  const std::string kDayMarkerLine = BuildDayMarkerLine(normalized_day_marker);
  if (!lines.empty() && Trim(lines.front()) == kDayMarkerLine) {
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

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
[[nodiscard]] auto BuildDayContentIsoDate(
    std::string_view selected_month, std::string_view normalized_day_marker)
    -> std::optional<std::string> {
  if (!IsValidDayMarker(normalized_day_marker)) {
    return std::nullopt;
  }
  const auto kParsedMonth = TryParseSelectedMonth(selected_month);
  if (!kParsedMonth.has_value()) {
    return std::nullopt;
  }

  const int kMarkerMonth =
      std::stoi(std::string(normalized_day_marker.substr(0, 2)));
  const int kMarkerDay =
      std::stoi(std::string(normalized_day_marker.substr(2, 2)));
  if (kMarkerMonth != kParsedMonth->month ||
      kMarkerDay > DaysInMonth(kParsedMonth->year, kParsedMonth->month)) {
    return std::nullopt;
  }
  return std::format("{:04d}-{:02d}-{:02d}", kParsedMonth->year, kMarkerMonth,
                     kMarkerDay);
}
// NOLINTEND(bugprone-easily-swappable-parameters)

}  // namespace

auto DefaultDayMarker(const DefaultTxtDayMarkerRequest& request)
    -> DefaultTxtDayMarkerResponse {
  const ParsedIsoDate kTargetDate = ParseIsoDate(request.target_date_iso);
  const auto kParsedMonth = TryParseSelectedMonth(request.selected_month);
  if (!kParsedMonth.has_value()) {
    return {.ok = true,
            .normalized_day_marker = kTargetDate.day_marker,
            .error_message = ""};
  }

  const int kMaxDay = DaysInMonth(kParsedMonth->year, kParsedMonth->month);
  return {
      .ok = true,
      .normalized_day_marker = std::format(
          "{:02d}{:02d}", kParsedMonth->month,
          std::min(kTargetDate.day, kMaxDay > 0 ? kMaxDay : kTargetDate.day)),
      .error_message = "",
  };
}

auto ResolveDayBlock(const ResolveTxtDayBlockRequest& request)
    -> ResolveTxtDayBlockResponse {
  const std::string kNormalizedDayMarker =
      NormalizeDayMarker(request.day_marker);
  const std::optional<std::string> kIsoDate =
      BuildDayContentIsoDate(request.selected_month, kNormalizedDayMarker);
  if (!IsValidDayMarker(kNormalizedDayMarker)) {
    return {.ok = true,
            .normalized_day_marker = kNormalizedDayMarker,
            .found = false,
            .is_marker_valid = false,
            .can_save = false,
            .day_body = "",
            .day_content_iso_date = kIsoDate,
            .error_message = ""};
  }

  const std::vector<std::string> kLines = SplitLines(request.content);
  const int kStartIndex = FindDayBlockStart(kLines, kNormalizedDayMarker);
  if (kStartIndex < 0) {
    return {.ok = true,
            .normalized_day_marker = kNormalizedDayMarker,
            .found = false,
            .is_marker_valid = true,
            .can_save = false,
            .day_body = "",
            .day_content_iso_date = kIsoDate,
            .error_message = ""};
  }

  const int kEndIndex = FindDayBlockEnd(kLines, kStartIndex);
  std::string day_body;
  for (int index = kStartIndex + 1; index < kEndIndex; ++index) {
    if (!day_body.empty()) {
      day_body.push_back('\n');
    }
    day_body.append(kLines[static_cast<std::size_t>(index)]);
  }

  return {.ok = true,
          .normalized_day_marker = kNormalizedDayMarker,
          .found = true,
          .is_marker_valid = true,
          .can_save = true,
          .day_body = day_body,
          .day_content_iso_date = kIsoDate,
          .error_message = ""};
}

auto ReplaceDayBlock(const ReplaceTxtDayBlockRequest& request)
    -> ReplaceTxtDayBlockResponse {
  const std::string kNormalizedDayMarker =
      NormalizeDayMarker(request.day_marker);
  if (!IsValidDayMarker(kNormalizedDayMarker)) {
    return {.ok = true,
            .normalized_day_marker = kNormalizedDayMarker,
            .found = false,
            .is_marker_valid = false,
            .updated_content = request.content,
            .error_message = ""};
  }

  std::vector<std::string> lines = SplitLines(request.content);
  const int kStartIndex = FindDayBlockStart(lines, kNormalizedDayMarker);
  if (kStartIndex < 0) {
    return {.ok = true,
            .normalized_day_marker = kNormalizedDayMarker,
            .found = false,
            .is_marker_valid = true,
            .updated_content = request.content,
            .error_message = ""};
  }

  const int kEndIndex = FindDayBlockEnd(lines, kStartIndex);
  const std::vector<std::string> kNormalizedBodyLines =
      NormalizeEditedDayBody(kNormalizedDayMarker, request.edited_day_body);
  lines.erase(lines.begin() + kStartIndex + 1, lines.begin() + kEndIndex);
  lines.insert(lines.begin() + kStartIndex + 1, kNormalizedBodyLines.begin(),
               kNormalizedBodyLines.end());

  return {.ok = true,
          .normalized_day_marker = kNormalizedDayMarker,
          .found = true,
          .is_marker_valid = true,
          .updated_content = JoinLinesCanonical(lines),
          .error_message = ""};
}

}  // namespace tracer::core::application::pipeline::txt_day_block
