// config_validator/queries/facade/QueryFacade.hpp
#ifndef QUERY_VALIDATOR_FACADE_HPP
#define QUERY_VALIDATOR_FACADE_HPP

#include <vector>
#include <string>
#include <nlohmann/json.hpp>

class QueryFacade {
public:
    bool validate(
        const std::vector<std::pair<std::string, nlohmann::json>>& query_configs
    ) const;
};

#endif // QUERY_VALIDATOR_FACADE_HPP