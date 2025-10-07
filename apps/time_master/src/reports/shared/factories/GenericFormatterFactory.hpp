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
#include "reports/shared/factories/DllFormatterWrapper.hpp" // [新增] 引入包装类

namespace fs = std::filesystem;

template<typename ReportDataType>
class GenericFormatterFactory {
public:
    using Creator = std::function<std::unique_ptr<IReportFormatter<ReportDataType>>(const AppConfig&)>;

    static std::unique_ptr<IReportFormatter<ReportDataType>> create(ReportFormat format, const AppConfig& config) {
        // ==================== [核心修改] ====================
        // 仅当数据类型是 DailyReportData 且格式是 Markdown 时，才执行DLL加载
        if constexpr (std::is_same_v<ReportDataType, DailyReportData>) {
            if (format == ReportFormat::Markdown) {
                try {
                    fs::path exe_dir(config.exe_dir_path);
                    fs::path plugin_dir = exe_dir / "plugins";

#ifdef _WIN32
                    // 在 Windows 上，动态库通常是 .dll 文件
                    // CMake 生成的库文件可能带有 "lib" 前缀
                    fs::path dll_path = plugin_dir / "libDayMdFormatter.dll";
                    if (!fs::exists(dll_path)) {
                        // 如果没有lib前缀，再试一次
                        dll_path = plugin_dir / "DayMdFormatter.dll";
                    }
#else
                    // 在 Linux 上是 .so，macOS 上是 .dylib
                    fs::path dll_path = plugin_dir / "libDayMdFormatter.so";
#endif
                    if (!fs::exists(dll_path)) {
                         throw std::runtime_error("Formatter plugin not found at: " + dll_path.string());
                    }

                    // 创建并返回包装器实例
                    return std::make_unique<DllFormatterWrapper<DailyReportData>>(dll_path.string(), config);

                } catch (const std::exception& e) {
                    std::cerr << "Error loading dynamic formatter: " << e.what() << std::endl;
                    // 如果加载失败，可以抛出异常或返回nullptr
                    throw;
                }
            }
        }
        // ====================================================

        // 对于所有其他情况，使用现有的静态注册逻辑
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