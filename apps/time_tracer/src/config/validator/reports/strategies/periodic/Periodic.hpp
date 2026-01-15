// config/validator/reports/strategies/periodic/Periodic.hpp
#ifndef PERIODIC_HPP
#define PERIODIC_HPP

#include "config/validator/reports/strategies/BaseStrategy.hpp"

class Periodic : public BaseStrategy {
protected:
    bool validate_specific_keys(const toml::table& query_config, const std::string& file_name) const override;
};

#endif // PERIODIC_HPP