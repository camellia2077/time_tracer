// infrastructure/platform/windows/windows_platform_clock.cpp
#include "infrastructure/platform/windows/windows_platform_clock.hpp"

#include <ctime>
#include <string>

namespace {
constexpr int kDateStringLength = 10;
constexpr int kYearWidth = 4;
constexpr int kMonthDayWidth = 2;
constexpr int kTmYearBase = 1900;
constexpr int kSecondsPerMinute = 60;

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void AppendPaddedNumber(std::string& output, int value, int width) {
  const std::string kDigits = std::to_string(value);
  if (kDigits.size() < static_cast<size_t>(width)) {
    output.append(static_cast<size_t>(width) - kDigits.size(), '0');
  }
  output += kDigits;
}

auto FormatDateTm(const std::tm& time_info) -> std::string {
  std::string output;
  output.reserve(kDateStringLength);
  AppendPaddedNumber(output, time_info.tm_year + kTmYearBase, kYearWidth);
  output += "-";
  AppendPaddedNumber(output, time_info.tm_mon + 1, kMonthDayWidth);
  output += "-";
  AppendPaddedNumber(output, time_info.tm_mday, kMonthDayWidth);
  return output;
}

auto ToLocalTm(std::time_t timestamp) -> std::tm {
  std::tm local_tm{};
#if defined(_WIN32) || defined(_WIN64)
  localtime_s(&local_tm, &timestamp);
#else
  localtime_r(&timestamp, &local_tm);
#endif
  return local_tm;
}

auto ToUtcTm(std::time_t timestamp) -> std::tm {
  std::tm utc_tm{};
#if defined(_WIN32) || defined(_WIN64)
  gmtime_s(&utc_tm, &timestamp);
#else
  gmtime_r(&timestamp, &utc_tm);
#endif
  return utc_tm;
}

}  // namespace

namespace infrastructure::platform {

auto WindowsPlatformClock::TodayLocalDateIso() const -> std::string {
  const std::time_t kNow = std::time(nullptr);
  return FormatDateTm(ToLocalTm(kNow));
}

auto WindowsPlatformClock::LocalUtcOffsetMinutes() const -> int {
  const std::time_t kNow = std::time(nullptr);
  std::tm local_tm = ToLocalTm(kNow);
  std::tm utc_tm = ToUtcTm(kNow);

  const std::time_t kLocalAsEpoch = std::mktime(&local_tm);
  const std::time_t kUtcAsLocalEpoch = std::mktime(&utc_tm);
  if (kLocalAsEpoch == static_cast<std::time_t>(-1) ||
      kUtcAsLocalEpoch == static_cast<std::time_t>(-1)) {
    return 0;
  }

  const auto kOffsetSeconds =
      static_cast<long long>(kLocalAsEpoch - kUtcAsLocalEpoch);
  return static_cast<int>(kOffsetSeconds / kSecondsPerMinute);
}

}  // namespace infrastructure::platform
