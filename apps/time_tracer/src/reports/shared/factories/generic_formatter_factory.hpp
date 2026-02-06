// reports/shared/factories/generic_formatter_factory.hpp
#ifndef REPORTS_SHARED_FACTORIES_GENERIC_FORMATTER_FACTORY_H_
#define REPORTS_SHARED_FACTORIES_GENERIC_FORMATTER_FACTORY_H_

#include <filesystem>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

#include "common/config/app_config.hpp"
#include "config/loader/toml_loader_utils.hpp"
#include "reports/shared/factories/dll_formatter_wrapper.hpp"
#include "reports/shared/interfaces/i_report_formatter.hpp"
#include "reports/shared/types/report_format.hpp"

namespace fs = std::filesystem;

template <typename ReportDataType>
class GenericFormatterFactory {
 public:
  using Creator =
      std::function<std::unique_ptr<IReportFormatter<ReportDataType>>(
          const AppConfig&)>;

  [[nodiscard]] static auto Create(ReportFormat format, const AppConfig& config)
      -> std::unique_ptr<IReportFormatter<ReportDataType>> {
    auto& creators = GetCreators();
    auto iter = creators.find(format);

    if (iter == creators.end()) {
      throw std::invalid_argument(
          "Unsupported report format or formatter not registered for this data "
          "type.");
    }

    return iter->second(config);
  }

  static void RegisterCreator(ReportFormat format, Creator creator) {
    GetCreators()[format] = std::move(creator);
  }

  static void RegisterDllFormatter(ReportFormat format,
                                   std::string dll_base_name) {
    RegisterCreator(
        format,
        [dll_base_name, format](const AppConfig& config)
            -> std::unique_ptr<IReportFormatter<ReportDataType>> {
          fs::path config_path;

          if constexpr (std::is_same_v<ReportDataType, DailyReportData>) {
            switch (format) {
              case ReportFormat::kMarkdown:
                config_path = config.reports.day_md_config_path;
                break;
              case ReportFormat::kLaTeX:
                config_path = config.reports.day_tex_config_path;
                break;
              case ReportFormat::kTyp:
                config_path = config.reports.day_typ_config_path;
                break;
            }
          } else if constexpr (std::is_same_v<ReportDataType,
                                              MonthlyReportData>) {
            switch (format) {
              case ReportFormat::kMarkdown:
                config_path = config.reports.month_md_config_path;
                break;
              case ReportFormat::kLaTeX:
                config_path = config.reports.month_tex_config_path;
                break;
              case ReportFormat::kTyp:
                config_path = config.reports.month_typ_config_path;
                break;
            }
          } else if constexpr (std::is_same_v<ReportDataType,
                                              PeriodReportData>) {
            switch (format) {
              case ReportFormat::kMarkdown:
                config_path = config.reports.period_md_config_path;
                break;
              case ReportFormat::kLaTeX:
                config_path = config.reports.period_tex_config_path;
                break;
              case ReportFormat::kTyp:
                config_path = config.reports.period_typ_config_path;
                break;
            }
          } else if constexpr (std::is_same_v<ReportDataType,
                                              WeeklyReportData>) {
            switch (format) {
              case ReportFormat::kMarkdown:
                config_path = config.reports.week_md_config_path;
                break;
              case ReportFormat::kLaTeX:
                config_path = config.reports.week_tex_config_path;
                break;
              case ReportFormat::kTyp:
                config_path = config.reports.week_typ_config_path;
                break;
            }
          } else if constexpr (std::is_same_v<ReportDataType,
                                              YearlyReportData>) {
            switch (format) {
              case ReportFormat::kMarkdown:
                config_path = config.reports.year_md_config_path;
                break;
              case ReportFormat::kLaTeX:
                config_path = config.reports.year_tex_config_path;
                break;
              case ReportFormat::kTyp:
                config_path = config.reports.year_typ_config_path;
                break;
            }
          }

          // [修改] 默认为空字符串，不再是 "{}"
          std::string config_content;
          if (!config_path.empty() && fs::exists(config_path)) {
            try {
              toml::table config_tbl = TomlLoaderUtils::ReadToml(config_path);
              std::stringstream buffer;
              buffer << config_tbl;
              config_content = buffer.str();
            } catch (const std::exception& e) {
              std::cerr << "Error reading config file: " << config_path << " - "
                        << e.what() << std::endl;
            }
          }

          // [修改] 传递 config_content
          return LoadFromDll(dll_base_name, config, config_content);
        });
  }

  static auto GetCreators() -> std::map<ReportFormat, Creator>& {
    static std::map<ReportFormat, Creator> creators;
    return creators;
  }

  [[nodiscard]] static auto LoadFromDll(const std::string& base_name,
                                        const AppConfig& config,
                                        const std::string& config_content)
      -> std::unique_ptr<IReportFormatter<ReportDataType>> {
    try {
      fs::path exe_dir(config.exe_dir_path);
      fs::path plugin_dir = exe_dir / "plugins";
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
          dll_path.string(), config_content);

    } catch (const std::exception& e) {
      std::cerr << "Error loading dynamic formatter: " << e.what() << std::endl;
      throw;
    }
  }
};

#endif  // REPORTS_SHARED_FACTORIES_GENERIC_FORMATTER_FACTORY_H_
