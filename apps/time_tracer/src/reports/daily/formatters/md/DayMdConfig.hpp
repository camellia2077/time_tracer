// reports/daily/formatters/md/DayMdConfig.hpp
#ifndef DAY_MD_CONFIG_HPP
#define DAY_MD_CONFIG_HPP
#include <filesystem>
#include "reports/daily/formatters/base/DayBaseConfig.hpp"

// DayMdConfig 现在直接继承 DayBaseConfig，无需添加额外成员
class DayMdConfig : public DayBaseConfig {
public:
    explicit DayMdConfig(const std::filesystem::path& config_path);
};

#endif // DAY_MD_CONFIG_HPP