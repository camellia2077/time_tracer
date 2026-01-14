// common/config/IConfigLoader.hpp
#ifndef I_CONFIG_LOADER_HPP
#define I_CONFIG_LOADER_HPP

/**
 * @brief 配置加载器抽象接口
 * @tparam T 需要被填充的配置类类型
 */
template <typename T>
class IConfigLoader {
public:
    virtual ~IConfigLoader() = default;

    /**
     * @brief 将配置数据加载到目标对象中
     * @param config_object 需要被填充的配置对象引用
     * @return 加载是否成功
     */
    virtual bool load(T& config_object) = 0;
};

#endif // I_CONFIG_LOADER_HPP