// common/config/i_config_loader.hpp
#ifndef COMMON_CONFIG_I_CONFIG_LOADER_H_
#define COMMON_CONFIG_I_CONFIG_LOADER_H_

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
  virtual bool Load(T& config_object) = 0;
};

#endif  // COMMON_CONFIG_I_CONFIG_LOADER_H_