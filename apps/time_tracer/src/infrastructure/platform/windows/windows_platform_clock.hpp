// infrastructure/platform/windows/windows_platform_clock.hpp
#ifndef INFRASTRUCTURE_PLATFORM_WINDOWS_PLATFORM_CLOCK_H_
#define INFRASTRUCTURE_PLATFORM_WINDOWS_PLATFORM_CLOCK_H_

#include "application/ports/i_platform_clock.hpp"

namespace infrastructure::platform {

class WindowsPlatformClock final
    : public time_tracer::application::ports::IPlatformClock {
 public:
  [[nodiscard]] auto TodayLocalDateIso() const -> std::string override;
  [[nodiscard]] auto LocalUtcOffsetMinutes() const -> int override;
};

}  // namespace infrastructure::platform

#endif  // INFRASTRUCTURE_PLATFORM_WINDOWS_PLATFORM_CLOCK_H_
