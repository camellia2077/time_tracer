#ifndef TRACER_CORE_TESTS_INFRASTRUCTURE_MODULES_SMOKE_SUPPORT_HPP_
#define TRACER_CORE_TESTS_INFRASTRUCTURE_MODULES_SMOKE_SUPPORT_HPP_

#include "application/ports/reporting/i_platform_clock.hpp"

#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>

class SmokePlatformClock final
    : public tracer_core::application::ports::IPlatformClock {
 public:
  [[nodiscard]] auto TodayLocalDateIso() const -> std::string override {
    return "2026-03-09";
  }

  [[nodiscard]] auto LocalUtcOffsetMinutes() const -> int override {
    return 8 * 60;
  }
};

inline auto WriteSmokeFile(const std::filesystem::path& path,
                           std::string_view content) -> void {
  std::filesystem::create_directories(path.parent_path());
  std::ofstream stream(path, std::ios::binary);
  stream << content;
}

#endif  // TRACER_CORE_TESTS_INFRASTRUCTURE_MODULES_SMOKE_SUPPORT_HPP_
