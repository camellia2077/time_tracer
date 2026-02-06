// cli/impl/utils/console_helper.cpp
#include "cli/impl/utils/console_helper.hpp"

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <shellapi.h>

namespace ConsoleHelper {

namespace {
auto WideToUtf8(const wchar_t* text) -> std::string {
  if (text == nullptr) {
    return {};
  }
  int required =
      WideCharToMultiByte(CP_UTF8, 0, text, -1, nullptr, 0, nullptr, nullptr);
  if (required <= 1) {
    return {};
  }
  std::string result(static_cast<size_t>(required - 1), '\0');
  WideCharToMultiByte(CP_UTF8, 0, text, -1, result.data(), required, nullptr,
                      nullptr);
  return result;
}
} // namespace

void SetupConsole() {
    SetConsoleOutputCP(CP_UTF8);
    HANDLE h_out = GetStdHandle(STD_OUTPUT_HANDLE);
    if (h_out != INVALID_HANDLE_VALUE) {
      DWORD dw_mode = 0;
      if (GetConsoleMode(h_out, &dw_mode) != 0) {
        dw_mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(h_out, dw_mode);
      }
    }
}

auto GetUtf8Args() -> std::vector<std::string> {
  int argc = 0;
  LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
  if (argv == nullptr || argc == 0) {
    return {};
  }
  std::vector<std::string> args;
  args.reserve(static_cast<size_t>(argc));
  for (int i = 0; i < argc; ++i) {
    args.emplace_back(WideToUtf8(argv[i]));
  }
  LocalFree(static_cast<void*>(argv));
  return args;
}

} // namespace ConsoleHelper
#endif
