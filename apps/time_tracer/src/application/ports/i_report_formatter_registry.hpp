// application/ports/i_report_formatter_registry.hpp
#ifndef APPLICATION_PORTS_I_REPORT_FORMATTER_REGISTRY_H_
#define APPLICATION_PORTS_I_REPORT_FORMATTER_REGISTRY_H_

#include <memory>
#include <string>
#include <vector>

namespace time_tracer::application::ports {

class IStaticReportFormatterRegistrar {
 public:
  virtual ~IStaticReportFormatterRegistrar() = default;

  virtual auto RegisterStaticFormatters() const -> void = 0;
};

class IReportFormatterRegistry {
 public:
  virtual ~IReportFormatterRegistry() = default;

  virtual auto RegisterFormatters() const -> void = 0;

  [[nodiscard]] virtual auto GetExpectedFormatterPluginNames() const
      -> std::vector<std::string> = 0;
};

// Default formatter registry provider. Implemented in infrastructure.
auto CreateReportFormatterRegistry(
    std::shared_ptr<IStaticReportFormatterRegistrar> static_registrar = nullptr)
    -> std::shared_ptr<IReportFormatterRegistry>;

}  // namespace time_tracer::application::ports

#endif  // APPLICATION_PORTS_I_REPORT_FORMATTER_REGISTRY_H_
