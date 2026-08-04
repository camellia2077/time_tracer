// domain/logic/validator/txt/rules/txt_rules.cpp
#include "domain/logic/validator/txt/rules/txt_rules.hpp"

#include <algorithm>
#include <array>

import tracer.core.shared.string_utils;

namespace validator::txt {

using tracer::core::shared::string_utils::Trim;

namespace {
constexpr int kMinMonth = 1;
constexpr int kMaxMonth = 12;

[[nodiscard]] auto IsAsciiDigit(char value) -> bool {
  return value >= '0' && value <= '9';
}

[[nodiscard]] auto IsValidAuthoredTime(std::string_view time) -> bool {
  constexpr size_t kLegacyTimeDigitsLength = 4;
  constexpr size_t kCanonicalTimeDigitsLength = 6;
  constexpr int kMaxHours = 23;
  constexpr int kMaxMinutes = 59;
  constexpr int kMaxSeconds = 59;
  if ((time.length() != kLegacyTimeDigitsLength &&
       time.length() != kCanonicalTimeDigitsLength) ||
      !std::ranges::all_of(
          time, [](char value) -> bool { return IsAsciiDigit(value); })) {
    return false;
  }

  try {
    const int kHours = std::stoi(std::string(time.substr(0, 2)));
    const int kMinutes = std::stoi(std::string(time.substr(2, 2)));
    if (kHours < 0 || kHours > kMaxHours || kMinutes < 0 ||
        kMinutes > kMaxMinutes) {
      return false;
    }
    return time.length() == kLegacyTimeDigitsLength ||
           std::stoi(std::string(time.substr(4, 2))) <= kMaxSeconds;
  } catch (const std::exception&) {
    return false;
  }
}

struct ParsedEventLine {
  std::string description;
};

auto ExtractRemarkAndDescription(std::string_view remaining_line)
    -> ParsedEventLine {
  size_t comment_pos = std::string::npos;
  constexpr std::array<const char*, 3> kDelimiters = {"//", "#", ";"};
  for (const char* delimiter : kDelimiters) {
    const size_t kPos = remaining_line.find(delimiter);
    if (kPos != std::string::npos &&
        (comment_pos == std::string::npos || kPos < comment_pos)) {
      comment_pos = kPos;
    }
  }

  return {.description =
              Trim(std::string(remaining_line.substr(0, comment_pos)))};
}
}  // namespace

LineRules::LineRules(const ConverterConfig& config) : config_(config) {
  for (const auto& entry : config.text_mapping) {
    valid_event_keywords_.insert(entry.first);
    valid_event_keywords_.insert(entry.second);
  }
  wake_keywords_.insert(config.sleep_inference.wake_keywords.begin(),
                        config.sleep_inference.wake_keywords.end());
  for (const auto& entry : config.top_parent_mapping) {
    valid_event_keywords_.insert(entry.first);
  }
  for (const auto& entry : config.initial_top_parents) {
    valid_event_keywords_.insert(entry.first);
  }
}

auto LineRules::IsYear(const std::string& line) -> bool {
  constexpr size_t kYearStringLength = 5;
  if (line.length() != kYearStringLength || line[0] != 'y') {
    return false;
  }
  return std::all_of(line.begin() + 1, line.end(), ::isdigit);
}

auto LineRules::IsMonth(const std::string& line) -> bool {
  constexpr size_t kMonthStringLength = 3;
  if (line.length() != kMonthStringLength || line[0] != 'm') {
    return false;
  }
  if (!std::all_of(line.begin() + 1, line.end(), ::isdigit)) {
    return false;
  }
  int month = 0;
  try {
    month = std::stoi(line.substr(1));
  } catch (const std::exception&) {
    return false;
  }
  return month >= kMinMonth && month <= kMaxMonth;
}

auto LineRules::IsDate(const std::string& line) -> bool {
  constexpr size_t kDateStringLength = 5;
  if (line.length() != kDateStringLength || line[0] != 'd') {
    return false;
  }
  return std::ranges::all_of(
      line.begin() + 1, line.end(),
      [](unsigned char kChar) -> bool { return std::isdigit(kChar) != 0; });
}

auto LineRules::IsRemark(const std::string& line) const -> bool {
  return line.starts_with("//") && !Trim(line.substr(2)).empty();
}

auto LineRules::IsValidEventLine(const std::string& line, int line_number,
                                 std::set<Error>& errors,
                                 const std::optional<SourceSpan>& span) const
    -> bool {
  constexpr size_t kMinimumEventLineLength = 5;
  constexpr size_t kLegacyTimeDigitsLength = 4;
  constexpr size_t kCanonicalTimeDigitsLength = 6;

  if (line.length() < kMinimumEventLineLength ||
      !std::ranges::all_of(
          line.substr(0, kLegacyTimeDigitsLength),
          [](char value) -> bool { return IsAsciiDigit(value); })) {
    return false;
  }
  std::string_view remaining_line;
  const bool kUsesSixDigitTime =
      line.length() >= kCanonicalTimeDigitsLength &&
      std::ranges::all_of(line.substr(0, kCanonicalTimeDigitsLength),
                          [](char value) { return IsAsciiDigit(value); });
  const size_t kTimeLength = kUsesSixDigitTime ? kCanonicalTimeDigitsLength
                                                 : kLegacyTimeDigitsLength;
  if (line.length() > kTimeLength && line[kTimeLength] == '-') {
    const size_t kEndOffset = kTimeLength + 1U;
    if (line.length() < kEndOffset + kTimeLength ||
        !IsValidAuthoredTime(std::string_view(line).substr(0, kTimeLength)) ||
        !IsValidAuthoredTime(
            std::string_view(line).substr(kEndOffset, kTimeLength))) {
      return false;
    }
    remaining_line = std::string_view(line).substr(kEndOffset + kTimeLength);
  } else {
    if (!IsValidAuthoredTime(std::string_view(line).substr(0, kTimeLength))) {
      return false;
    }
    remaining_line = std::string_view(line).substr(kTimeLength);
  }

  const ParsedEventLine kParsed = ExtractRemarkAndDescription(remaining_line);
  if (kParsed.description.empty()) {
    return false;
  }

  if (!wake_keywords_.contains(kParsed.description) &&
      !valid_event_keywords_.contains(kParsed.description)) {
    // Unknown activity is a semantic validation error, not a syntax error:
    // the line is structurally valid but references unmapped domain terms.
    errors.insert({line_number,
                   "Unrecognized activity '" + kParsed.description +
                       "'. Please check spelling or update config file.",
                   ErrorType::kUnrecognizedActivity, span});
  }
  return true;
}

void StructureRules::Reset() {
  has_seen_year_ = false;
  has_seen_date_in_block_ = false;
  has_seen_event_in_day_ = false;
  has_seen_any_date_ = false;
  has_seen_month_ = false;
  has_reported_missing_month_header_ = false;
  month_header_.clear();
  last_seen_year_ = 0;
}

void StructureRules::ProcessYearLine(int line_number, const std::string& line,
                                     std::set<Error>& errors,
                                     const SourceSpan& span) {
  if (has_seen_year_) {
    errors.insert({line_number,
                   "Multiple year headers found. Only one year header is "
                   "allowed per file (single month/year per file).",
                   ErrorType::kStructural, span});
    return;
  }

  int current_year = 0;
  try {
    current_year = std::stoi(line.substr(1));
  } catch (const std::exception&) {
    errors.insert(
        {line_number, "Invalid year format.", ErrorType::kStructural, span});
    return;
  }

  has_seen_year_ = true;
  last_seen_year_ = current_year;
  has_seen_month_ = false;
  has_reported_missing_month_header_ = false;
  month_header_.clear();
  has_seen_any_date_ = false;
  has_seen_date_in_block_ = false;
}

void StructureRules::ProcessMonthLine(int line_number, const std::string& line,
                                      std::set<Error>& errors,
                                      const SourceSpan& span) {
  if (!has_seen_year_) {
    errors.insert({line_number, "Month header found before a year header.",
                   ErrorType::kStructural, span});
    return;
  }

  if (has_seen_month_) {
    errors.insert({line_number,
                   "Multiple month headers found. Only one month header "
                   "(mMM) is allowed per file.",
                   ErrorType::kStructural, span});
    return;
  }

  if (has_seen_any_date_) {
    errors.insert({line_number,
                   "Month header (mMM) must appear before the first date "
                   "line (dMMDD).",
                   ErrorType::kStructural, span});
    return;
  }

  int month = 0;
  try {
    month = std::stoi(line.substr(1));
  } catch (const std::exception&) {
    errors.insert({line_number, "Invalid month header format.",
                   ErrorType::kStructural, span});
    return;
  }

  if (month < kMinMonth || month > kMaxMonth) {
    errors.insert({line_number, "Month header out of range. Use m01..m12.",
                   ErrorType::kStructural, span});
    return;
  }

  has_seen_month_ = true;
  month_header_ = line.substr(1);
}

void StructureRules::ProcessDateLine(int line_number, const std::string& line,
                                     std::set<Error>& errors,
                                     const SourceSpan& span) {
  if (!has_seen_year_) {
    errors.insert({line_number, "Date found before a year header.",
                   ErrorType::kStructural, span});
  }

  if (!has_seen_month_ && !has_reported_missing_month_header_) {
    errors.insert({line_number,
                   "Month header (mMM) is required before date lines.",
                   ErrorType::kStructural, span});
    has_reported_missing_month_header_ = true;
  }

  if (has_seen_month_) {
    const std::string kLineMonth = line.substr(1, 2);
    if (kLineMonth != month_header_) {
      errors.insert({line_number,
                     "Date month '" + kLineMonth +
                         "' does not match month header '" + month_header_ +
                         "'.",
                     ErrorType::kStructural, span});
    }
  }

  // Date continuity/completeness is a mode-controlled business check. It is
  // intentionally not enforced by the TXT structural parser, because hosts
  // such as Android allow a user's first record to begin on any day of the
  // month. StructValidator applies this rule when continuity/full is enabled.
  has_seen_any_date_ = true;

  has_seen_date_in_block_ = true;
  has_seen_event_in_day_ = false;
}

void StructureRules::ProcessRemarkLine(int line_number,
                                       const std::string& /*line*/,
                                       std::set<Error>& errors,
                                       const SourceSpan& span) const {
  if (!has_seen_date_in_block_) {
    errors.insert({line_number, "Remark found before a date.",
                   ErrorType::kStructural, span});
  }
  if (has_seen_event_in_day_) {
    errors.insert({line_number,
                   "Remark must appear before any events for the day.",
                   ErrorType::kSourceRemarkAfterEvent, span});
  }
}

void StructureRules::ProcessEventLine(int line_number,
                                      const std::string& /*line*/,
                                      std::set<Error>& errors,
                                      const SourceSpan& span) {
  if (!has_seen_date_in_block_) {
    errors.insert({line_number, "Event found before a date.",
                   ErrorType::kStructural, span});
  }
  has_seen_event_in_day_ = true;
}

void StructureRules::ProcessUnrecognizedLine(int line_number,
                                             const std::string& line,
                                             std::set<Error>& errors,
                                             const SourceSpan& span) {
  errors.insert({line_number, "Unrecognized line format: " + line,
                 ErrorType::kSourceInvalidLineFormat, span});
}

auto StructureRules::HasSeenYear() const -> bool {
  return has_seen_year_;
}

auto StructureRules::HasSeenMonth() const -> bool {
  return has_seen_month_;
}

auto StructureRules::HasSeenEventInDay() const -> bool {
  return has_seen_event_in_day_;
}

}  // namespace validator::txt
