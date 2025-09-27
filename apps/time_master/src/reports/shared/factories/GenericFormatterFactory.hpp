// reports/shared/factories/GenericFormatterFactory.hpp
#ifndef GENERIC_FORMATTER_FACTORY_HPP
#define GENERIC_FORMATTER_FACTORY_HPP

#include <functional>
#include <map>
#include <memory>
#include <stdexcept>
#include "reports/shared/types/ReportFormat.hpp"
#include "common/AppConfig.hpp"
#include "reports/shared/interfaces/IReportFormatter.hpp"

template<typename ReportDataType>
class GenericFormatterFactory {
public:
    // 定义一个创建者函数的类型别名
    using Creator = std::function<std::unique_ptr<IReportFormatter<ReportDataType>>(const AppConfig&)>;

    /**
     * @brief 根据指定的格式创建一个格式化器实例。
     * @param format 报告格式。
     * @param config 应用程序配置。
     * @return 一个指向创建的格式化器的 unique_ptr。
     * @throws std::invalid_argument 如果格式不被支持。
     */
    static std::unique_ptr<IReportFormatter<ReportDataType>> create(ReportFormat format, const AppConfig& config) {
        auto& creators = get_creators();
        auto it = creators.find(format);
        if (it == creators.end()) {
            throw std::invalid_argument("Unsupported report format for the given data type.");
        }
        return it->second(config);
    }

    /**
     * @brief 注册一个新的格式化器创建者。
     * @param format 要注册的报告格式。
     * @param creator 用于创建格式化器实例的 lambda 函数或函数对象。
     */
    static void regist(ReportFormat format, Creator creator) {
        get_creators()[format] = std::move(creator);
    }

private:
    /**
     * @brief 获取存储创建者的静态 map 的引用。
     * 使用函数静态变量确保线程安全的初始化。
     * @return 一个从 ReportFormat 映射到创建者函数的 map。
     */
    static std::map<ReportFormat, Creator>& get_creators() {
        static std::map<ReportFormat, Creator> creators;
        return creators;
    }
};

#endif // GENERIC_FORMATTER_FACTORY_HPP