// domain/logic/validator/txt/rules/txt_rules.cpp
#include "domain/logic/validator/txt/rules/txt_rules.hpp"

#include <algorithm>
#include <array>

#include "shared/utils/string_utils.hpp"

namespace validator::txt {

namespace {
constexpr int kMinMonth = 1;
constexpr int kMaxMonth = 12;
}  // namespace

LineRules::LineRules(const ConverterConfig& config) : config_(config) {
  for (const auto& entry : config.text_mapping) {
    valid_event_keywords_.insert(entry.first);
  }
  for (const auto& entry : config.text_duration_mapping) {
    valid_event_keywords_.insert(entry.first);
  }
  wake_keywords_.insert(config.wake_keywords.begin(),
                        config.wake_keywords.end());
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
  constexpr size_t kDateStringLength = 4;
  return line.length() == kDateStringLength &&
         std::ranges::all_of(line, [](unsigned char kChar) -> bool {
           return std::isdigit(kChar) != 0;
         });
}

auto LineRules::IsRemark(const std::string& line) const -> bool {
  const std::string& prefix = config_.remark_prefix;
  if (prefix.empty() || !line.starts_with(prefix)) {
    return false;
  }
  return !Trim(line.substr(prefix.length())).empty();
}

auto LineRules::IsValidEventLine(const std::string& line, int line_number,
                                 std::set<Error>& errors,
                                 const std::optional<SourceSpan>& span) const
    -> bool {
  constexpr size_t kMinimumEventLineLength = 5;
  constexpr size_t kTimePrefixLength = 4;
  constexpr int kMaxHours = 23;
  constexpr int kMaxMinutes = 59;

  if (line.length() < kMinimumEventLineLength ||
      !std::all_of(line.begin(), line.begin() + kTimePrefixLength, ::isdigit)) {
    return false;
  }
  try {
    int hours = std::stoi(line.substr(0, 2));
    int minutes = std::stoi(line.substr(2, 2));
    if (hours > kMaxHours || minutes > kMaxMinutes) {
      return false;
    }

    const std::string kRemainingLine = line.substr(kTimePrefixLength);
    size_t comment_pos = std::string::npos;
    constexpr std::array<const char*, 3> kDelimiters = {"//", "#", ";"};
    for (const char* delimiter : kDelimiters) {
      size_t pos = kRemainingLine.find(delimiter);
      if (pos != std::string::npos &&
          (comment_pos == std::string::npos || pos < comment_pos)) {
        comment_pos = pos;
      }
    }

    const std::string kDescription =
        Trim(kRemainingLine.substr(0, comment_pos));
    if (kDescription.empty()) {
      return false;
    }

    if (!wake_keywords_.contains(kDescription) &&
        !valid_event_keywords_.contains(kDescription)) {
      errors.insert({line_number,
                     "Unrecognized activity '" + kDescription +
                         "'. Please check spelling or update config file.",
                     ErrorType::kUnrecognizedActivity, span});
    }
    return true;
  } catch (const std::exception&) {
    return false;
  }
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
                   "line (MMDD).",
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
    const std::string kLineMonth = line.substr(0, 2);
    if (kLineMonth != month_header_) {
      errors.insert({line_number,
                     "Date month '" + kLineMonth +
                         "' does not match month header '" + month_header_ +
                         "'.",
                     ErrorType::kStructural, span});
    }
  }

  if (!has_seen_any_date_) {
    if (line.length() >= 4) {
      std::string day_part = line.substr(2, 2);
      if (day_part != "01") {
        errors.insert({line_number,
                       "The first date in the file must be the 1st day of the "
                       "month (e.g., 0101). Found: " +
                           line,
                       ErrorType::kDateContinuity, span});
      }
    }
    has_seen_any_date_ = true;
  }

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

}  // namespace validator::txt
