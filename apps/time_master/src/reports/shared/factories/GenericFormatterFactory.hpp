// reports/shared/factories/GenericFormatterFactory.hpp
#ifndef GENERIC_FORMATTER_FACTORY_HPP
#define GENERIC_FORMATTER_FACTORY_HPP

#include <functional>
#include <map>
#include <memory>
#include <stdexcept>
#include <filesystem>
#include "reports/shared/types/ReportFormat.hpp"
#include "common/AppConfig.hpp"
#include "reports/shared/interfaces/IReportFormatter.hpp"
#include "reports/shared/factories/DllFormatterWrapper.hpp"

namespace fs = std::filesystem;

template<typename ReportDataType>
class GenericFormatterFactory {
public:
    using Creator = std::function<std::unique_ptr<IReportFormatter<ReportDataType>>(const AppConfig&)>;

    static std::unique_ptr<IReportFormatter<ReportDataType>> create(ReportFormat format, const AppConfig& config) {
        // --- 使用 std::map 进行重构，代替 if-else if ---
        std::string base_name;
        bool is_dll_format = false;

        // --- 日报 DLL 映射 ---
        if constexpr (std::is_same_v<ReportDataType, DailyReportData>) {
            static const std::map<ReportFormat, std::string> daily_format_map = {
                {ReportFormat::Markdown, "DayMdFormatter"},
                {ReportFormat::Typ,      "DayTypFormatter"},
                {ReportFormat::LaTeX,    "DayTexFormatter"}
            };
            auto it = daily_format_map.find(format);
            if (it != daily_format_map.end()) {
                base_name = it->second;
                is_dll_format = true;
            }
        }
        
        // --- 月报 DLL 映射 ---
        if constexpr (std::is_same_v<ReportDataType, MonthlyReportData>) {
            static const std::map<ReportFormat, std::string> monthly_format_map = {
                {ReportFormat::Markdown, "MonthMdFormatter"},
                {ReportFormat::Typ,      "MonthTypFormatter"},
                {ReportFormat::LaTeX,    "MonthTexFormatter"}
            };
            auto it = monthly_format_map.find(format);
            if (it != monthly_format_map.end()) {
                base_name = it->second;
                is_dll_format = true;
            }
        }

        // --- 通用的 DLL 加载逻辑 ---
        if (is_dll_format) {
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
                     throw std::runtime_error("Formatter plugin not found at: " + dll_path.string());
                }

                // 根据模板参数实例化正确的包装器
                return std::make_unique<DllFormatterWrapper<ReportDataType>>(dll_path.string(), config);

            } catch (const std::exception& e) {
                std::cerr << "Error loading dynamic formatter: " << e.what() << std::endl;
                throw;
            }
        }

        // --- 回退到静态注册的格式化器 ---
        auto& creators = get_creators();
        auto it = creators.find(format);
        if (it == creators.end()) {
            throw std::invalid_argument("Unsupported report format for the given data type.");
        }
        return it->second(config);
    }

    static void regist(ReportFormat format, Creator creator) {
        get_creators()[format] = std::move(creator);
    }

private:
    static std::map<ReportFormat, Creator>& get_creators() {
        static std::map<ReportFormat, Creator> creators;
        return creators;
    }
};

#endif // GENERIC_FORMATTER_FACTORY_HPP