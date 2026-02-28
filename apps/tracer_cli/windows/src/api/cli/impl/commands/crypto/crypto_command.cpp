// api/cli/impl/commands/crypto/crypto_command.cpp
#include <filesystem>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "api/cli/framework/core/command_catalog.hpp"
#include "api/cli/framework/core/command_parser.hpp"
#include "api/cli/framework/core/command_registry.hpp"
#include "api/cli/framework/core/command_validator.hpp"
#include "api/cli/framework/interfaces/i_command.hpp"
#include "api/cli/impl/app/app_context.hpp"
#include "api/cli/impl/commands/crypto/crypto_file_runner.hpp"
#include "api/cli/impl/commands/crypto/crypto_passphrase_io.hpp"
#include "application/use_cases/i_tracer_core_api.hpp"
#include "domain/types/date_check_mode.hpp"

class CryptoCommand : public ICommand {
public:
  CryptoCommand(ITracerCoreApi &core_api, DateCheckMode default_date_check_mode)
      : core_api_(core_api), default_date_check_mode_(default_date_check_mode) {
  }

  [[nodiscard]] auto GetDefinitions() const -> std::vector<ArgDef> override;
  [[nodiscard]] auto GetHelp() const -> std::string override;
  [[nodiscard]] auto GetCategory() const -> std::string override {
    return tracer_core::cli::framework::core::ResolveCommandCategory(
        "crypto", ICommand::GetCategory());
  }
  auto Execute(const CommandParser &parser) -> void override;

private:
  ITracerCoreApi &core_api_;
  DateCheckMode default_date_check_mode_;
};

namespace {

namespace fs = std::filesystem;
namespace crypto_runner = tracer_core::cli::impl::commands::crypto;

} // namespace

namespace tracer_core::cli::impl::commands {

void RegisterCryptoCommand() {
  CommandRegistry<AppContext>::Instance().RegisterCommand(
      "crypto", [](AppContext &ctx) -> std::unique_ptr<CryptoCommand> {
        if (!ctx.core_api) {
          throw std::runtime_error("Core API not initialized");
        }
        return std::make_unique<CryptoCommand>(
            *ctx.core_api, ctx.config.default_date_check_mode);
      });
}

} // namespace tracer_core::cli::impl::commands

auto CryptoCommand::GetDefinitions() const -> std::vector<ArgDef> {
  return {{"action",
           ArgType::kPositional,
           {},
           "Crypto action (encrypt, decrypt, inspect)",
           true,
           "",
           0},
          {"input",
           ArgType::kOption,
           {"--in"},
           "Input file or directory path",
           true,
           ""},
          {"output",
           ArgType::kOption,
           {"--out"},
           "Output file or directory path (required for encrypt/decrypt)",
           false,
           ""},
          {"security_level",
           ArgType::kOption,
           {"--security-level"},
           "Encryption security level (interactive, moderate, high)",
           false,
           ""}};
}

auto CryptoCommand::GetHelp() const -> std::string {
  return "Encrypt/decrypt/inspect raw TXT transfer files (.txt <-> .tracer).\n"
         "\n"
         "  Usage:\n"
         "    crypto encrypt --in <txt|dir> --out <tracer|dir> "
         "[--security-level interactive|moderate|high]\n"
         "    crypto decrypt --in <tracer|dir> --out <txt|dir>\n"
         "    crypto inspect --in <tracer|dir>\n"
         "\n"
         "  Notes:\n"
         "    - Encrypt runs validate-structure + validate-logic before file "
         "encryption.\n"
         "    - Passphrase input is mandatory and interactive (not from CLI "
         "arguments).\n"
         "    - Compression is fixed to zstd level 1 in current phase.\n"
         "    - --security-level defaults to interactive.";
}

void CryptoCommand::Execute(const CommandParser &parser) {
  ParsedArgs args = CommandValidator::Validate(parser, GetDefinitions());
  const auto action = crypto_runner::ParseCryptoAction(args.Get("action"));
  if (!action.has_value()) {
    throw std::runtime_error("Invalid action. Use: encrypt, decrypt, inspect.");
  }

  const fs::path input_path = fs::absolute(fs::path(args.Get("input")));
  if (input_path.empty()) {
    throw std::runtime_error("--in is required.");
  }
  if (!fs::exists(input_path)) {
    throw std::runtime_error("Input path does not exist: " +
                             input_path.string());
  }

  if (action == crypto_runner::CryptoAction::kInspect) {
    crypto_runner::RunCryptoInspect(input_path);
    return;
  }

  if (!args.Has("output")) {
    throw std::runtime_error("--out is required for encrypt/decrypt.");
  }
  const fs::path output_path = fs::absolute(fs::path(args.Get("output")));

  if (action == crypto_runner::CryptoAction::kEncrypt) {
    const std::string security_level_value =
        args.Has("security_level") ? args.Get("security_level") : "interactive";
    const std::optional<crypto_runner::CryptoSecurityLevel> security_level =
        crypto_runner::ParseCryptoSecurityLevel(security_level_value);
    if (!security_level.has_value()) {
      throw std::runtime_error(
          "Invalid --security-level. Use: interactive, moderate, high.");
    }
    const std::string passphrase = crypto_runner::PromptPassphraseForEncrypt();
    crypto_runner::RunCryptoEncrypt(core_api_, default_date_check_mode_,
                                    input_path, output_path, passphrase,
                                    security_level.value());
    return;
  }

  const std::string passphrase = crypto_runner::PromptPassphraseForDecrypt();
  crypto_runner::RunCryptoDecrypt(input_path, output_path, passphrase);
}
