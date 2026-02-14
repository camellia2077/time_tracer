// application/ports/i_platform_clock.hpp
#ifndef APPLICATION_PORTS_I_PLATFORM_CLOCK_H_
#define APPLICATION_PORTS_I_PLATFORM_CLOCK_H_

#include <string>

namespace time_tracer::application::ports {

class IPlatformClock {
 public:
  virtual ~IPlatformClock() = default;

  [[nodiscard]] virtual auto TodayLocalDateIso() const -> std::string = 0;
  [[nodiscard]] virtual auto LocalUtcOffsetMinutes() const -> int = 0;
};

}  // namespace time_tracer::application::ports

#endif  // APPLICATION_PORTS_I_PLATFORM_CLOCK_H_
