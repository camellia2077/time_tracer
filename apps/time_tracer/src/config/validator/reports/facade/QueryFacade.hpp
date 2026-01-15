// config/validator/reports/facade/QueryFacade.hpp
#ifndef QUERY_FACADE_HPP
#define QUERY_FACADE_HPP

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

#endif // QUERY_FACADE_HPP