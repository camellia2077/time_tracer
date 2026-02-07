// main_cli.cpp
#include <string>
#include <vector>

#include "api/cli/impl/app/app_runner.hpp"
#include "api/cli/impl/utils/console_helper.hpp"
#include "shared/types/ansi_colors.hpp"

using namespace ConsoleHelper;

auto main(int argc, char* argv[]) -> int {
  try {
#if defined(_WIN32) || defined(_WIN64)
    SetupConsole();
    (void)argc;
    (void)argv;
    std::vector<std::string> args = GetUtf8Args();
#else
    std::vector<std::string> args(argv, argv + argc);
#endif

    return AppRunner::Run(std::move(args));

  } catch (const std::exception& e) {
    SafePrintln(stderr, "{}Startup Error: {}{}",
                time_tracer::common::colors::kRed, e.what(),
                time_tracer::common::colors::kReset);
    return 1;
  } catch (...) {
    SafePrintln(stderr, "{}Startup Error: An unknown exception occurred.{}",
                time_tracer::common::colors::kRed,
                time_tracer::common::colors::kReset);
    return 1;
  }
}
