// infrastructure/reports/shared/factories/generic_formatter_factory.hpp
#ifndef INFRASTRUCTURE_REPORTS_SHARED_FACTORIES_GENERIC_FORMATTER_FACTORY_H_
#define INFRASTRUCTURE_REPORTS_SHARED_FACTORIES_GENERIC_FORMATTER_FACTORY_H_

#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>

#include "domain/ports/diagnostics.hpp"
#include "domain/reports/types/report_types.hpp"
#include "infrastructure/config/models/report_catalog.hpp"
#include "infrastructure/reports/shared/factories/dll_formatter_wrapper.hpp"
#include "infrastructure/reports/shared/factories/formatter_config_payload.hpp"
#include "infrastructure/reports/shared/interfaces/i_report_formatter.hpp"

namespace fs = std::filesystem;

namespace generic_formatter_factory_detail {

enum class ReportDataKind {
  kDaily,
  kMonthly,
  kPeriod,
  kWeekly,
  kYearly,
  kRange,
};

[[nodiscard]] auto ResolveFormatterConfigKind(ReportDataKind report_data_kind,
                                              ReportFormat format) -> uint32_t;

[[nodiscard]] auto BuildConfigPayloadFromLoaded(ReportDataKind report_data_kind,
                                                ReportFormat format,
                                                const ReportCatalog& catalog)
    -> FormatterConfigPayload;

}  // namespace generic_formatter_factory_detail

template <typename ReportDataType>
class GenericFormatterFactory {
 public:
  using Creator =
      std::function<std::unique_ptr<IReportFormatter<ReportDataType>>(
          const ReportCatalog&)>;

  [[nodiscard]] static auto Create(ReportFormat format,
                                   const ReportCatalog& catalog)
      -> std::unique_ptr<IReportFormatter<ReportDataType>> {
    auto& creators = GetCreators();
    auto iter = creators.find(format);

    if (iter == creators.end()) {
      throw std::invalid_argument(
          "Unsupported report format or formatter not registered for this data "
          "type.");
    }

    return iter->second(catalog);
  }

  static void RegisterCreator(ReportFormat format, Creator creator) {
    GetCreators()[format] = std::move(creator);
  }

  static void RegisterDllFormatter(ReportFormat format,
                                   std::string dll_base_name) {
    RegisterCreator(
        format,
        [dll_base_name, format](const ReportCatalog& catalog)
            -> std::unique_ptr<IReportFormatter<ReportDataType>> {
          const uint32_t kConfigKind = GetFormatterConfigKind(format);
          if (kConfigKind == TT_FORMATTER_CONFIG_KIND_UNKNOWN) {
            throw std::invalid_argument(
                "Unsupported formatter config kind for selected report "
                "format.");
          }
          FormatterConfigPayload config_payload =
              BuildConfigPayloadFromLoaded(format, catalog);

          // Pass payload by const reference all the way through.
          // The ABI config view points to payload-owned string buffers, so a
          // move here may invalidate pointer addresses before DLL init.
          return LoadFromDll(dll_base_name, catalog, config_payload);
        });
  }

  static auto GetCreators() -> std::map<ReportFormat, Creator>& {
    static std::map<ReportFormat, Creator> creators;
    return creators;
  }

 private:
  template <typename T>
  static constexpr bool kAlwaysFalse = false;

  [[nodiscard]] static consteval auto ResolveReportDataKind()
      -> generic_formatter_factory_detail::ReportDataKind {
    using Kind = generic_formatter_factory_detail::ReportDataKind;
    if constexpr (std::is_same_v<ReportDataType, DailyReportData>) {
      return Kind::kDaily;
    } else if constexpr (std::is_same_v<ReportDataType, MonthlyReportData>) {
      return Kind::kMonthly;
    } else if constexpr (std::is_same_v<ReportDataType, PeriodReportData>) {
      return Kind::kPeriod;
    } else if constexpr (std::is_same_v<ReportDataType, WeeklyReportData>) {
      return Kind::kWeekly;
    } else if constexpr (std::is_same_v<ReportDataType, YearlyReportData>) {
      return Kind::kYearly;
    } else if constexpr (std::is_same_v<ReportDataType, RangeReportData>) {
      return Kind::kRange;
    } else {
      static_assert(kAlwaysFalse<ReportDataType>,
                    "Unsupported ReportDataType for GenericFormatterFactory.");
    }
  }

 public:
  [[nodiscard]] static auto GetFormatterConfigKind(ReportFormat format)
      -> uint32_t {
    return generic_formatter_factory_detail::ResolveFormatterConfigKind(
        ResolveReportDataKind(), format);
  }

  [[nodiscard]] static auto BuildConfigPayloadFromLoaded(
      ReportFormat format, const ReportCatalog& catalog)
      -> FormatterConfigPayload {
    return generic_formatter_factory_detail::BuildConfigPayloadFromLoaded(
        ResolveReportDataKind(), format, catalog);
  }

  // Must take const reference, not by value:
  // moving FormatterConfigPayload can break C-view string pointer stability.
  [[nodiscard]] static auto LoadFromDll(
      const std::string& base_name, const ReportCatalog& catalog,
      const FormatterConfigPayload& config_payload)
      -> std::unique_ptr<IReportFormatter<ReportDataType>> {
    try {
      fs::path plugin_dir = catalog.plugin_dir_path;
      fs::path dll_path;

#ifdef _WIN32
      dll_path = plugin_dir / ("lib" + base_name + ".dll");
      if (!fs::exists(dll_path)) {
        dll_path = plugin_dir / (base_name + ".dll");
      }
#else
      dll_path = plugin_dir / ("lib" + base_name + ".so");
#endif
      if (!fs::exists(dll_path)) {
        throw std::runtime_error("Formatter plugin not found at: " +
                                 dll_path.string());
      }

      return std::make_unique<DllFormatterWrapper<ReportDataType>>(
          dll_path.string(), config_payload);

    } catch (const std::exception& exception) {
      tracer_core::domain::ports::EmitError(
          "Error loading dynamic formatter: " + std::string(exception.what()));
      throw;
    }
  }
};

#endif  // INFRASTRUCTURE_REPORTS_SHARED_FACTORIES_GENERIC_FORMATTER_FACTORY_H_
