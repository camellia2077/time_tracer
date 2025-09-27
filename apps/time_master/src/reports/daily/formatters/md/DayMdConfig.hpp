// reports/daily/formatters/md/DayMdConfig.hpp
#ifndef DAY_MD_CONFIG_HPP
#define DAY_MD_CONFIG_HPP

#include "reports/daily/formatters/base/DayBaseConfig.hpp" // [修改] 引入基类

// DayMdConfig 现在直接继承 DayBaseConfig，无需添加额外成员
class DayMdConfig : public DayBaseConfig {
public:
    // 构造函数直接调用基类的构造函数
    explicit DayMdConfig(const std::string& config_path);
};

#endif // DAY_MD_CONFIG_HPP