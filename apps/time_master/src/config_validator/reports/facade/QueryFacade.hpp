// config_validator/reports/facade/QueryFacade.hpp
#ifndef QUERY_FACADE_HPP
#define QUERY_FACADE_HPP

#include <vector>
#include <string>
#include <nlohmann/json.hpp>

class QueryFacade {
public:
    bool validate(
        const std::vector<std::pair<std::string, nlohmann::json>>& query_configs
    ) const;
};

#endif // QUERY_FACADE_HPP