// api/cli/impl/commands/crypto/crypto_passphrase_io.cpp
#include "api/cli/impl/commands/crypto/crypto_passphrase_io.hpp"

#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

#if defined(_WIN32) || defined(_WIN64)
#include <conio.h>
#include <io.h>
#endif

namespace tracer_core::cli::impl::commands::crypto {
namespace {

[[nodiscard]] auto ReadHiddenLine(std::string_view prompt) -> std::string {
  std::cout << prompt;
  std::string value;

#if defined(_WIN32) || defined(_WIN64)
  const int stdin_fd = _fileno(stdin);
  if (stdin_fd >= 0 && _isatty(stdin_fd) == 0) {
    if (!std::getline(std::cin, value)) {
      throw std::runtime_error("Failed to read passphrase from stdin.");
    }
    return value;
  }

  for (;;) {
    const int ch = _getch();
    if (ch == '\r' || ch == '\n') {
      std::cout << '\n';
      break;
    }
    if (ch == 8) {
      if (!value.empty()) {
        value.pop_back();
      }
      continue;
    }
    if (ch == 3) {
      throw std::runtime_error("Passphrase input interrupted.");
    }
    value.push_back(static_cast<char>(ch));
  }
#else
  if (!std::getline(std::cin, value)) {
    throw std::runtime_error("Failed to read passphrase from stdin.");
  }
#endif
  return value;
}

} // namespace

auto PromptPassphraseForEncrypt() -> std::string {
  const std::string passphrase_1 =
      ReadHiddenLine("Enter passphrase (required): ");
  if (passphrase_1.empty()) {
    throw std::runtime_error("Passphrase is required.");
  }
  const std::string passphrase_2 = ReadHiddenLine("Confirm passphrase: ");
  if (passphrase_1 != passphrase_2) {
    throw std::runtime_error("Passphrase confirmation does not match.");
  }
  return passphrase_1;
}

auto PromptPassphraseForDecrypt() -> std::string {
  const std::string passphrase =
      ReadHiddenLine("Enter passphrase (required): ");
  if (passphrase.empty()) {
    throw std::runtime_error("Passphrase is required.");
  }
  return passphrase;
}

} // namespace tracer_core::cli::impl::commands::crypto
