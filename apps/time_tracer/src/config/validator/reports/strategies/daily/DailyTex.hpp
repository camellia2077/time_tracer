// config/validator/reports/strategies/daily/DailyTex.hpp
#ifndef DAILY_TEX_HPP
#define DAILY_TEX_HPP

#include "config/validator/reports/strategies/BaseStrategy.hpp"

class DailyTex : public BaseStrategy {
protected:
    bool validate_specific_keys(const toml::table& query_config, const std::string& file_name) const override;
};

#endif // DAILY_TEX_HPP