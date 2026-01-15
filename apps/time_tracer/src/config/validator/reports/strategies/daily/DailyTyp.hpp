// config/validator/reports/strategies/daily/DailyTyp.hpp
#ifndef DAILY_TYP_HPP
#define DAILY_TYP_HPP

#include "config/validator/reports/strategies/BaseStrategy.hpp"

class DailyTyp : public BaseStrategy {
protected:
    bool validate_specific_keys(const toml::table& query_config, const std::string& file_name) const override;
};

#endif // DAILY_TYP_HPP