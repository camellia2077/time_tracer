// config/validator/reports/facade/QueryFacade.hpp
#ifndef CONFIG_VALIDATOR_REPORTS_FACADE_QUERY_FACADE_HPP_
#define CONFIG_VALIDATOR_REPORTS_FACADE_QUERY_FACADE_HPP_

#include <vector>
#include <string>
#include <toml++/toml.h> // [修改]

class QueryFacade {
public:
    // [修改] 参数类型改为 toml::table
    bool validate(
        const std::vector<std::pair<std::string, toml::table>>& query_configs
    ) const;
};

#endif // CONFIG_VALIDATOR_REPORTS_FACADE_QUERY_FACADE_HPP_