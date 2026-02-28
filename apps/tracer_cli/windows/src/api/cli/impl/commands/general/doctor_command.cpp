// api/cli/impl/commands/general/doctor_command.cpp
#include <filesystem>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "api/cli/framework/core/command_catalog.hpp"
#include "api/cli/framework/core/command_parser.hpp"
#include "api/cli/framework/core/command_registry.hpp"
#include "api/cli/framework/core/command_validator.hpp"
#include "api/cli/framework/interfaces/i_command.hpp"
#include "api/cli/impl/app/app_context.hpp"
#include "shared/types/ansi_colors.hpp"
#include "shared/types/exceptions.hpp"

namespace fs = std::filesystem;

class DoctorCommand : public ICommand {
public:
  [[nodiscard]] auto GetDefinitions() const -> std::vector<ArgDef> override;
  [[nodiscard]] auto GetHelp() const -> std::string override;
  [[nodiscard]] auto GetCategory() const -> std::string override {
    return tracer_core::cli::framework::core::ResolveCommandCategory(
        "doctor", ICommand::GetCategory());
  }

  auto Execute(const CommandParser &parser) -> void override;
};

namespace {

struct DoctorPaths {
  fs::path exe_path;
  fs::path exe_dir;
  fs::path output_root;
  fs::path db_path;
};

struct DoctorCheck {
  std::string id;
  bool required = true;
  bool ok = false;
  fs::path path;
  std::string detail;
};

[[nodiscard]] auto ToDisplayPath(const fs::path &path) -> std::string {
  return path.lexically_normal().string();
}

[[nodiscard]] auto CountPluginDlls(const fs::path &plugins_dir) -> int {
  if (!fs::exists(plugins_dir) || !fs::is_directory(plugins_dir)) {
    return 0;
  }
  int count = 0;
  for (const auto &entry : fs::directory_iterator(plugins_dir)) {
    if (!entry.is_regular_file()) {
      continue;
    }
    if (entry.path().extension() == ".dll") {
      ++count;
    }
  }
  return count;
}

[[nodiscard]] auto BuildPaths(const CommandParser &parser) -> DoctorPaths {
  DoctorPaths paths{};
  paths.exe_path = fs::absolute(parser.GetRawArg(0));
  paths.exe_dir = paths.exe_path.parent_path();
  const auto output_override = parser.GetOption({"-o", "--output"});
  const auto db_override = parser.GetOption({"--db", "--database"});

  paths.output_root = output_override.has_value()
                          ? fs::absolute(*output_override)
                          : (paths.exe_dir / "output");
  paths.db_path = db_override.has_value()
                      ? fs::absolute(*db_override)
                      : (paths.output_root / "db" / "time_data.sqlite3");
  return paths;
}

[[nodiscard]] auto BuildChecks(const DoctorPaths &paths)
    -> std::vector<DoctorCheck> {
  std::vector<DoctorCheck> checks;

  const fs::path core_dll = paths.exe_dir / "tracer_core.dll";
  checks.push_back(DoctorCheck{
      .id = "core_dll",
      .required = true,
      .ok = fs::exists(core_dll),
      .path = core_dll,
      .detail = fs::exists(core_dll) ? "present" : "missing",
  });

  const fs::path reports_dll = paths.exe_dir / "libreports_shared.dll";
  checks.push_back(DoctorCheck{
      .id = "reports_shared_dll",
      .required = true,
      .ok = fs::exists(reports_dll),
      .path = reports_dll,
      .detail = fs::exists(reports_dll) ? "present" : "missing",
  });

  const fs::path config_toml = paths.exe_dir / "config" / "config.toml";
  checks.push_back(DoctorCheck{
      .id = "config_toml",
      .required = true,
      .ok = fs::exists(config_toml),
      .path = config_toml,
      .detail = fs::exists(config_toml) ? "present" : "missing",
  });

  const fs::path converter_alias =
      paths.exe_dir / "config" / "converter" / "alias_mapping.toml";
  checks.push_back(DoctorCheck{
      .id = "converter_alias_mapping",
      .required = true,
      .ok = fs::exists(converter_alias),
      .path = converter_alias,
      .detail = fs::exists(converter_alias) ? "present" : "missing",
  });

  const fs::path plugins_dir = paths.exe_dir / "plugins";
  const bool plugins_dir_exists =
      fs::exists(plugins_dir) && fs::is_directory(plugins_dir);
  checks.push_back(DoctorCheck{
      .id = "plugins_dir",
      .required = true,
      .ok = plugins_dir_exists,
      .path = plugins_dir,
      .detail = plugins_dir_exists ? "present" : "missing",
  });

  const int plugin_dll_count = CountPluginDlls(plugins_dir);
  checks.push_back(DoctorCheck{
      .id = "plugins_dll_count",
      .required = true,
      .ok = plugin_dll_count > 0,
      .path = plugins_dir,
      .detail = "count=" + std::to_string(plugin_dll_count),
  });

  const fs::path db_parent = paths.db_path.parent_path();
  checks.push_back(DoctorCheck{
      .id = "db_parent_dir",
      .required = false,
      .ok = fs::exists(db_parent) && fs::is_directory(db_parent),
      .path = db_parent,
      .detail = (fs::exists(db_parent) && fs::is_directory(db_parent))
                    ? "present"
                    : "missing",
  });

  checks.push_back(DoctorCheck{
      .id = "db_file",
      .required = false,
      .ok = fs::exists(paths.db_path),
      .path = paths.db_path,
      .detail = fs::exists(paths.db_path) ? "present" : "missing",
  });

  return checks;
}

void PrintDoctorText(const DoctorPaths &paths,
                     const std::vector<DoctorCheck> &checks,
                     int required_passed, int required_total) {
  namespace colors = tracer_core::common::colors;

  std::cout << colors::kBold << "Runtime Doctor" << colors::kReset << "\n";
  std::cout << "  exe_path   : " << ToDisplayPath(paths.exe_path) << "\n";
  std::cout << "  exe_dir    : " << ToDisplayPath(paths.exe_dir) << "\n";
  std::cout << "  output_root: " << ToDisplayPath(paths.output_root) << "\n";
  std::cout << "  db_path    : " << ToDisplayPath(paths.db_path) << "\n\n";

  for (const auto &check : checks) {
    const std::string status = check.ok ? "[OK]   " : "[MISS] ";
    std::cout << "  " << status << check.id << " -> "
              << ToDisplayPath(check.path) << " (" << check.detail << ")";
    if (!check.required) {
      std::cout << " [optional]";
    }
    std::cout << "\n";
  }

  const bool all_required_ok = required_passed == required_total;
  std::cout << "\n  Summary: required " << required_passed << "/"
            << required_total << (all_required_ok ? " passed." : " failed.")
            << "\n";
}

[[nodiscard]] auto IsRequiredFailure(const DoctorCheck &check) -> bool {
  return check.required && !check.ok;
}

[[nodiscard]] auto HasCheckFailure(const std::vector<DoctorCheck> &checks,
                                   std::string_view id) -> bool {
  for (const auto &check : checks) {
    if (check.id == id && IsRequiredFailure(check)) {
      return true;
    }
  }
  return false;
}

} // namespace

namespace tracer_core::cli::impl::commands {

void RegisterDoctorCommand() {
  CommandRegistry<AppContext>::Instance().RegisterCommand(
      "doctor", [](AppContext &) -> std::unique_ptr<DoctorCommand> {
        return std::make_unique<DoctorCommand>();
      });
}

} // namespace tracer_core::cli::impl::commands

auto DoctorCommand::GetDefinitions() const -> std::vector<ArgDef> {
  return {
      {"output",
       ArgType::kOption,
       {"-o", "--output"},
       "Output root override used to resolve db path",
       false,
       ""},
      {"db",
       ArgType::kOption,
       {"--db", "--database"},
       "Database path override used by runtime",
       false,
       ""},
  };
}

auto DoctorCommand::GetHelp() const -> std::string {
  return tracer_core::cli::framework::core::ResolveCommandSummary(
      "doctor", ICommand::GetHelp());
}

auto DoctorCommand::Execute(const CommandParser &parser) -> void {
  CommandValidator::Validate(parser, GetDefinitions());

  const DoctorPaths paths = BuildPaths(parser);
  const std::vector<DoctorCheck> checks = BuildChecks(paths);

  int required_total = 0;
  int required_passed = 0;
  for (const auto &check : checks) {
    if (!check.required) {
      continue;
    }
    ++required_total;
    if (check.ok) {
      ++required_passed;
    }
  }
  const bool all_required_ok = required_passed == required_total;

  PrintDoctorText(paths, checks, required_passed, required_total);

  if (all_required_ok) {
    return;
  }

  if (HasCheckFailure(checks, "config_toml") ||
      HasCheckFailure(checks, "converter_alias_mapping")) {
    throw tracer_core::common::ConfigError(
        "Doctor check failed: missing required config files.");
  }

  if (HasCheckFailure(checks, "core_dll") ||
      HasCheckFailure(checks, "reports_shared_dll") ||
      HasCheckFailure(checks, "plugins_dir") ||
      HasCheckFailure(checks, "plugins_dll_count")) {
    throw tracer_core::common::DllCompatibilityError(
        "Doctor check failed: missing required runtime dependencies.");
  }

  throw tracer_core::common::IoError(
      "Doctor check failed: runtime path checks are not healthy.");
}
