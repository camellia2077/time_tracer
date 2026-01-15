// config/validator/reports/strategies/monthly/Monthly.hpp
#ifndef MONTHLY_HPP
#define MONTHLY_HPP

#include "config/validator/reports/strategies/BaseStrategy.hpp"

class Monthly : public BaseStrategy {
protected:
    bool validate_specific_keys(const toml::table& query_config, const std::string& file_name) const override;
};

#endif // MONTHLY_HPP