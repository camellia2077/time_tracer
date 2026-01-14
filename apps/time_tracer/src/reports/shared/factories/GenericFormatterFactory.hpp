// reports/shared/factories/GenericFormatterFactory.hpp
#ifndef GENERIC_FORMATTER_FACTORY_HPP
#define GENERIC_FORMATTER_FACTORY_HPP

#include <functional>
#include <map>
#include <memory>
#include <stdexcept>
#include <filesystem>
#include <iostream>
#include <string>

#include "reports/shared/types/ReportFormat.hpp"
#include "common/AppConfig.hpp"
#include "reports/shared/interfaces/IReportFormatter.hpp"
#include "reports/shared/factories/DllFormatterWrapper.hpp"

namespace fs = std::filesystem;

template<typename ReportDataType>
class GenericFormatterFactory {
public:
    using Creator = std::function<std::unique_ptr<IReportFormatter<ReportDataType>>(const AppConfig&)>;

    /**
     * @brief 创建格式化器实例。
     * 现在它不再包含硬编码逻辑，而是直接从注册表中查找。
     */
    static std::unique_ptr<IReportFormatter<ReportDataType>> create(ReportFormat format, const AppConfig& config) {
        auto& creators = get_creators();
        auto it = creators.find(format);
        
        if (it == creators.end()) {
            throw std::invalid_argument("Unsupported report format or formatter not registered for this data type.");
        }
        
        return it->second(config);
    }

    /**
     * @brief 注册一个自定义的创建函数（适用于静态链接的格式化器）。
     */
    static void register_creator(ReportFormat format, Creator creator) {
        get_creators()[format] = std::move(creator);
    }

    /**
     * @brief [新增] 注册基于 DLL 的格式化器。
     * 这是对 register_creator 的封装，专门用于处理动态库加载逻辑。
     * * @param format 报告格式枚举
     * @param dll_base_name DLL 的基础名称（不带 .dll/.so 后缀和 lib 前缀），例如 "DayMdFormatter"
     */
    static void register_dll_formatter(ReportFormat format, std::string dll_base_name) {
        // 注册一个 lambda，当 create 被调用时，这个 lambda 会执行并加载 DLL
        register_creator(format, [dll_base_name](const AppConfig& config) {
            return load_from_dll(dll_base_name, config);
        });
    }

private:
    // 单例注册表
    static std::map<ReportFormat, Creator>& get_creators() {
        static std::map<ReportFormat, Creator> creators;
        return creators;
    }

    // [重构] 将 DLL 路径查找和加载逻辑提取为私有辅助函数
    static std::unique_ptr<IReportFormatter<ReportDataType>> load_from_dll(const std::string& base_name, const AppConfig& config) {
        try {
            fs::path exe_dir(config.exe_dir_path);
            fs::path plugin_dir = exe_dir / "plugins";
            fs::path dll_path;

#ifdef _WIN32
            // 尝试加载 libName.dll 或 Name.dll
            dll_path = plugin_dir / ("lib" + base_name + ".dll");
            if (!fs::exists(dll_path)) {
                dll_path = plugin_dir / (base_name + ".dll");
            }
#else
            // Linux/Unix 假设为 libName.so
            dll_path = plugin_dir / ("lib" + base_name + ".so");
#endif
            if (!fs::exists(dll_path)) {
                 throw std::runtime_error("Formatter plugin not found at: " + dll_path.string());
            }

            return std::make_unique<DllFormatterWrapper<ReportDataType>>(dll_path.string(), config);

        } catch (const std::exception& e) {
            std::cerr << "Error loading dynamic formatter: " << e.what() << std::endl;
            throw;
        }
    }
};

#endif // GENERIC_FORMATTER_FACTORY_HPP