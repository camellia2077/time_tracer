// application/config/config_handler.hpp
#ifndef APPLICATION_CONFIG_CONFIG_HANDLER_H_
#define APPLICATION_CONFIG_CONFIG_HANDLER_H_

#include <filesystem>
#include <optional>

#include "application/ports/file_system.hpp"
#include "common/app_context.hpp"
#include "common/config_types.hpp"

namespace App {

class ConfigHandler {
 public:
  explicit ConfigHandler(FileSystem& file_system);
  std::optional<Core::AppContext> load(const Config& config,
                                       const std::filesystem::path& exe_dir);

 private:
  FileSystem& file_system_;
};

}  // namespace App

#endif  // APPLICATION_CONFIG_CONFIG_HANDLER_H_
