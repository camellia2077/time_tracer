// config/validator/reports/strategies/daily/DailyMd.hpp
#ifndef DAILY_MD_HPP
#define DAILY_MD_HPP

#include "config/validator/reports/strategies/BaseStrategy.hpp"

class DailyMd : public BaseStrategy {
protected:
    bool validate_specific_keys(const toml::table& query_config, const std::string& file_name) const override;
};

#endif // DAILY_MD_HPP