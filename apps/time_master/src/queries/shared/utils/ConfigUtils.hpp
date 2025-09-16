// queries/shared/utils/ConfigUtils.hpp
#ifndef CONFIG_UTILS_HPP
#define CONFIG_UTILS_HPP

#include <nlohmann/json.hpp>
#include <string>

/**
 * @brief 从指定路径加载并解析 JSON 配置文件。
 * @param config_path 配置文件的路径。
 * @param error_message_prefix 当文件无法打开时，用于构造异常信息的前缀。
 * @return 解析后的 nlohmann::json 对象。
 * @throws std::runtime_error 如果无法打开或解析文件。
 */
nlohmann::json load_json_config(const std::string& config_path, const std::string& error_message_prefix);

#endif // CONFIG_UTILS_HPP