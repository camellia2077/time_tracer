// reports/daily/formatters/md/DayMdConfig.cpp
#include "DayMdConfig.hpp"


// 构造函数只需调用基类的构造函数即可。
// 所有通用的配置加载逻辑都已在 DayBaseConfig 中完成。

// 保留这个文件的原因，是因为，如果未来给 Markdown 添加特有的配置项
// 就不需要再去修改 DayMd 的签名，直接在这个类里加就行了
DayMdConfig::DayMdConfig(const std::filesystem::path& config_path)
    : DayBaseConfig(config_path) {
}