// config/validator/reports/facade/query_facade.hpp
#ifndef CONFIG_VALIDATOR_REPORTS_FACADE_QUERY_FACADE_HPP_
#define CONFIG_VALIDATOR_REPORTS_FACADE_QUERY_FACADE_HPP_

#include <vector>
#include <string>
#include <toml++/toml.h> 

class QueryFacade {
public:
    bool validate(
        const std::vector<std::pair<std::string, toml::table>>& query_configs
    ) const;
};

#endif // CONFIG_VALIDATOR_REPORTS_FACADE_QUERY_FACADE_HPP_