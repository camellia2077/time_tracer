// infrastructure/config/config_facade.hpp
#ifndef INFRASTRUCTURE_CONFIG_CONFIG_FACADE_H_
#define INFRASTRUCTURE_CONFIG_CONFIG_FACADE_H_

#include <string>
#include <vector>

#include "infrastructure/config/config.hpp"

namespace ConfigFacade {
bool validate(const TomlConfigData& config_data,
              std::vector<std::string>& errors);
}

#endif  // INFRASTRUCTURE_CONFIG_CONFIG_FACADE_H_
