// application/application.cpp
#include "application/application.hpp"

#include "application/config/config_handler.hpp"
#include "application/reporting/report_handler.hpp"
#include "application/workflow/workflow_handler.hpp"
#include "utils/utils.hpp"

namespace App {

Application::Application(FileSystem& file_system,
                         ILogGeneratorFactory& generator_factory)
    : file_system_(file_system), generator_factory_(generator_factory) {}

auto Application::run(const Config& config,
                      const std::filesystem::path& exe_dir) -> ExitCode {
  Utils::setup_console();

  ConfigHandler config_handler(file_system_);
  auto context_opt = config_handler.load(config, exe_dir);
  if (!context_opt) {
    return ExitCode::kRuntimeConfigLoadFailed;
  }

  ReportHandler report_handler;
  WorkflowHandler workflow(file_system_, generator_factory_);

  int files_generated = workflow.run(*context_opt, report_handler);
  if (files_generated < 0) {
    return ExitCode::kGenerationFailed;
  }

  report_handler.finish(context_opt->config, files_generated);
  return ExitCode::kSuccess;
}
}  // namespace App
