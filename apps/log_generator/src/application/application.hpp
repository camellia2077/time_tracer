// application/application.hpp
#ifndef APPLICATION_APPLICATION_H_
#define APPLICATION_APPLICATION_H_

#include <filesystem>

#include "application/ports/file_system.hpp"
#include "application/ports/log_generator_factory.hpp"
#include "common/config_types.hpp"
#include "common/exit_code.hpp"

namespace App {
class Application {
 public:
  Application(FileSystem& file_system, ILogGeneratorFactory& generator_factory);
  ExitCode run(const Config& config, const std::filesystem::path& exe_dir);

 private:
  FileSystem& file_system_;
  ILogGeneratorFactory& generator_factory_;
};
}  // namespace App

#endif  // APPLICATION_APPLICATION_H_
