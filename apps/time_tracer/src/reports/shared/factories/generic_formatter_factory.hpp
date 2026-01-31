// reports/shared/factories/generic_formatter_factory.hpp
#ifndef REPORTS_SHARED_FACTORIES_GENERIC_FORMATTER_FACTORY_H_
#define REPORTS_SHARED_FACTORIES_GENERIC_FORMATTER_FACTORY_H_

#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

#include "common/config/app_config.hpp"
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

  static std::unique_ptr<IReportFormatter<ReportDataType>> create(
      ReportFormat format, const AppConfig& config) {
    auto& creators = get_creators();
    auto it = creators.find(format);

    if (it == creators.end()) {
      throw std::invalid_argument(
          "Unsupported report format or formatter not registered for this data "
          "type.");
    }

    return it->second(config);
  }

  static void register_creator(ReportFormat format, Creator creator) {
    get_creators()[format] = std::move(creator);
  }

  static void register_dll_formatter(ReportFormat format,
                                     std::string dll_base_name) {
    register_creator(format, [dll_base_name, format](const AppConfig& config) {
      fs::path config_path;

      if constexpr (std::is_same_v<ReportDataType, DailyReportData>) {
        switch (format) {
          case ReportFormat::Markdown:
            config_path = config.reports.day_md_config_path;
            break;
          case ReportFormat::LaTeX:
            config_path = config.reports.day_tex_config_path;
            break;
          case ReportFormat::Typ:
            config_path = config.reports.day_typ_config_path;
            break;
        }
      } else if constexpr (std::is_same_v<ReportDataType, MonthlyReportData>) {
        switch (format) {
          case ReportFormat::Markdown:
            config_path = config.reports.month_md_config_path;
            break;
          case ReportFormat::LaTeX:
            config_path = config.reports.month_tex_config_path;
            break;
          case ReportFormat::Typ:
            config_path = config.reports.month_typ_config_path;
            break;
        }
      } else if constexpr (std::is_same_v<ReportDataType, PeriodReportData>) {
        switch (format) {
          case ReportFormat::Markdown:
            config_path = config.reports.period_md_config_path;
            break;
          case ReportFormat::LaTeX:
            config_path = config.reports.period_tex_config_path;
            break;
          case ReportFormat::Typ:
            config_path = config.reports.period_typ_config_path;
            break;
        }
      }

      // [修改] 默认为空字符串，不再是 "{}"
      std::string config_content = "";
      if (!config_path.empty() && fs::exists(config_path)) {
        try {
          std::ifstream file(config_path);
          if (file) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            config_content = buffer.str();
          }
        } catch (const std::exception& e) {
          std::cerr << "Error reading config file: " << config_path << " - "
                    << e.what() << std::endl;
        }
      }

      // [修改] 传递 config_content
      return load_from_dll(dll_base_name, config, config_content);
    });
  }

 private:
  static std::map<ReportFormat, Creator>& get_creators() {
    static std::map<ReportFormat, Creator> creators;
    return creators;
  }

  static std::unique_ptr<IReportFormatter<ReportDataType>> load_from_dll(
      const std::string& base_name, const AppConfig& config,
      const std::string& config_content) {
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