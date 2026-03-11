#ifndef TRACER_CORE_TESTS_INFRASTRUCTURE_MODULES_SMOKE_SUPPORT_HPP_
#define TRACER_CORE_TESTS_INFRASTRUCTURE_MODULES_SMOKE_SUPPORT_HPP_

#include "application/interfaces/i_report_exporter.hpp"
#include "application/ports/i_platform_clock.hpp"
#include "application/ports/logger.hpp"
#include "domain/types/converter_config.hpp"
#include "infrastructure/config/models/app_config.hpp"
#include "infrastructure/config/models/report_catalog.hpp"
#include "infrastructure/config/models/report_config_models.hpp"

#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

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

auto RunInfrastructureModuleLoggingPlatformConfigSmoke() -> int;
auto RunInfrastructureModuleQueryStatsRepositorySmoke() -> int;
auto RunInfrastructureModuleQueryInternalOrchestratorsSmoke() -> int;
auto RunInfrastructureModulePersistenceSmoke() -> int;
auto RunInfrastructureModuleReportsSmoke() -> int;

#endif  // TRACER_CORE_TESTS_INFRASTRUCTURE_MODULES_SMOKE_SUPPORT_HPP_
