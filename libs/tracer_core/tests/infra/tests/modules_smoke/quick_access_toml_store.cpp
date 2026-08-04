import tracer.core.infrastructure.config;

#include "application/ports/config/quick_access_toml_store.hpp"
#include "infra/tests/modules_smoke/config.hpp"

#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace {

namespace config = tracer::core::application::config;

auto Check(bool condition, std::string_view message, int& failures) -> void {
  if (!condition) {
    ++failures;
    std::cerr << "[FAIL] " << message << '\n';
  }
}

template <typename Function>
auto Throws(Function&& function) -> bool {
  try {
    function();
  } catch (const std::exception&) {
    return true;
  }
  return false;
}

auto ExpectReadFailure(std::string_view content, std::string_view message,
                       int& failures) -> void {
  Check(Throws([&]() {
          static_cast<void>(config::ParseQuickAccessToml(content));
        }),
        message, failures);
}

}  // namespace

auto RunQuickAccessTomlStoreTests() -> int {
  int failures = 0;
  try {
    const std::vector<std::string> expected = {"学习", "休息", "read"};
    const auto content =
        config::RenderQuickAccessToml(config::QuickAccessConfig{expected});
    const auto loaded = config::ParseQuickAccessToml(content);
    Check(loaded.aliases == expected,
          "Quick Access aliases must preserve order and Unicode", failures);

    Check(Throws([&]() {
            static_cast<void>(config::RenderQuickAccessToml(
                config::QuickAccessConfig{{"valid", ""}}));
          }),
          "empty alias must be rejected", failures);
    Check(Throws([&]() {
            static_cast<void>(config::RenderQuickAccessToml(
                config::QuickAccessConfig{{" valid"}}));
          }),
          "leading whitespace must be rejected", failures);
    Check(Throws([&]() {
            static_cast<void>(config::RenderQuickAccessToml(
                config::QuickAccessConfig{{"valid "}}));
          }),
          "trailing whitespace must be rejected", failures);
    Check(Throws([&]() {
            static_cast<void>(config::RenderQuickAccessToml(
                config::QuickAccessConfig{{"same", "same"}}));
          }),
          "duplicate alias must be rejected", failures);

    ExpectReadFailure("quick_access = \"not-an-array\"\n",
                      "non-array Quick Access field must be rejected",
                      failures);
    ExpectReadFailure("other = []\n",
                      "missing Quick Access field must be rejected", failures);
    ExpectReadFailure("quick_access = [\"ok\"]\nother = true\n",
                      "unknown Quick Access field must be rejected", failures);
    ExpectReadFailure("quick_access = [\"unterminated]\n",
                      "malformed Quick Access TOML must be rejected", failures);
  } catch (const std::exception& exception) {
    ++failures;
    std::cerr << "[FAIL] Quick Access TOML test threw: " << exception.what()
              << '\n';
  }

  if (failures == 0) {
    std::cout << "[PASS] quick_access_toml_store\n";
  }
  return failures;
}
