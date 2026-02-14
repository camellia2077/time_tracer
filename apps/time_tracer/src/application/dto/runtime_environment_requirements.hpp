// application/dto/runtime_environment_requirements.hpp
#ifndef APPLICATION_DTO_RUNTIME_ENVIRONMENT_REQUIREMENTS_H_
#define APPLICATION_DTO_RUNTIME_ENVIRONMENT_REQUIREMENTS_H_

#include <filesystem>
#include <string>
#include <vector>

namespace time_tracer::application::dto {

struct RuntimeEnvironmentRequirements {
  std::filesystem::path plugins_directory;
  std::filesystem::path binary_directory;
  std::vector<std::string> expected_formatter_plugins;
  std::vector<std::string> required_core_runtime_libraries;
};

}  // namespace time_tracer::application::dto

#endif  // APPLICATION_DTO_RUNTIME_ENVIRONMENT_REQUIREMENTS_H_
