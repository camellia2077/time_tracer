// application/ports/i_platform_clock.hpp
#ifndef APPLICATION_PORTS_I_PLATFORM_CLOCK_H_
#define APPLICATION_PORTS_I_PLATFORM_CLOCK_H_

#include <string>

namespace tracer_core::application::ports {

class IPlatformClock {
 public:
  virtual ~IPlatformClock() = default;

  [[nodiscard]] virtual auto TodayLocalDateIso() const -> std::string = 0;
  [[nodiscard]] virtual auto LocalUtcOffsetMinutes() const -> int = 0;
};

}  // namespace tracer_core::application::ports

#endif  // APPLICATION_PORTS_I_PLATFORM_CLOCK_H_
