#include "domain/formatting/event_line_formatter.hpp"

#include <array>

namespace {
constexpr int kMinutesPerHour = 60;
constexpr int kHoursPerDay = 24;
constexpr int kMinutesPerDay = kHoursPerDay * kMinutesPerHour;

constexpr std::array<std::string_view, 60> kDigits = {
    "00", "01", "02", "03", "04", "05", "06", "07", "08", "09", "10", "11",
    "12", "13", "14", "15", "16", "17", "18", "19", "20", "21", "22", "23",
    "24", "25", "26", "27", "28", "29", "30", "31", "32", "33", "34", "35",
    "36", "37", "38", "39", "40", "41", "42", "43", "44", "45", "46", "47",
    "48", "49", "50", "51", "52", "53", "54", "55", "56", "57", "58", "59"};

auto ToMinuteOfDay(int logical_minutes) -> int {
  int minute_of_day = logical_minutes % kMinutesPerDay;
  if (minute_of_day < 0) {
    minute_of_day += kMinutesPerDay;
  }
  return minute_of_day;
}

void AppendTime(std::string& buffer, int logical_minutes,
                TimeFormat time_format) {
  const int minute_of_day = ToMinuteOfDay(logical_minutes);
  const int hour = minute_of_day / kMinutesPerHour;
  const int minute = minute_of_day % kMinutesPerHour;
  buffer.append(kDigits[hour]);
  buffer.append(kDigits[minute]);
  if (time_format == TimeFormat::Hhmmss) {
    buffer.append("00");
  }
}

void AppendRemarkSuffix(std::string& buffer,
                        const std::optional<std::string>& remark_suffix) {
  if (remark_suffix.has_value()) {
    buffer.append(*remark_suffix);
  }
}
}  // namespace

namespace EventLineFormatter {

auto format_point_event_line(int end_minute, std::string_view activity_token,
                             const std::optional<std::string>& remark_suffix,
                             TimeFormat time_format)
    -> std::string {
  std::string line;
  line.reserve(16 + activity_token.size() +
               (remark_suffix.has_value() ? remark_suffix->size() : 0U));
  AppendTime(line, end_minute, time_format);
  line.append(activity_token);
  AppendRemarkSuffix(line, remark_suffix);
  return line;
}

auto format_interval_event_line(
    int start_minute, int end_minute, std::string_view activity_token,
    const std::optional<std::string>& remark_suffix, TimeFormat time_format)
    -> std::string {
  std::string line;
  line.reserve(24 + activity_token.size() +
               (remark_suffix.has_value() ? remark_suffix->size() : 0U));
  AppendTime(line, start_minute, time_format);
  line.push_back('-');
  AppendTime(line, end_minute, time_format);
  line.append(activity_token);
  AppendRemarkSuffix(line, remark_suffix);
  return line;
}

void append_formatted_event(std::string& buffer, const GeneratedEvent& event,
                            TimeFormat time_format) {
  if (event.kind == GeneratedEventKind::Interval) {
    buffer.append(format_interval_event_line(event.start_minute, event.end_minute,
                                             event.activity_token,
                                             event.remark_suffix, time_format));
    return;
  }

  buffer.append(
      format_point_event_line(event.end_minute, event.activity_token,
                              event.remark_suffix, time_format));
}

}  // namespace EventLineFormatter
