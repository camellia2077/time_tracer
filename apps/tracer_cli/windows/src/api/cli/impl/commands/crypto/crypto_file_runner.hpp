// api/cli/impl/commands/crypto/crypto_file_runner.hpp
#pragma once

#include <filesystem>
#include <optional>
#include <string_view>

#include "application/use_cases/i_tracer_core_api.hpp"
#include "domain/types/date_check_mode.hpp"

namespace tracer_core::cli::impl::commands::crypto {

enum class CryptoAction { kEncrypt, kDecrypt, kInspect };
enum class CryptoSecurityLevel { kInteractive, kModerate, kHigh };

[[nodiscard]] auto ParseCryptoAction(std::string_view action)
    -> std::optional<CryptoAction>;
[[nodiscard]] auto ParseCryptoSecurityLevel(std::string_view value)
    -> std::optional<CryptoSecurityLevel>;

auto RunCryptoEncrypt(ITracerCoreApi &core_api, DateCheckMode date_check_mode,
                      const std::filesystem::path &input_path,
                      const std::filesystem::path &output_path,
                      std::string_view passphrase,
                      CryptoSecurityLevel security_level) -> void;

auto RunCryptoDecrypt(const std::filesystem::path &input_path,
                      const std::filesystem::path &output_path,
                      std::string_view passphrase) -> void;

auto RunCryptoInspect(const std::filesystem::path &input_path) -> void;

} // namespace tracer_core::cli::impl::commands::crypto
