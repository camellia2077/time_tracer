// reports/daily/formatters/md/DayMdConfig.cpp
#include "DayMdConfig.hpp"

// 构造函数只需调用基类的构造函数即可。
// 所有通用的配置加载逻辑都已在 DayBaseConfig 中完成。
DayMdConfig::DayMdConfig(const std::string& config_path)
    : DayBaseConfig(config_path) {}