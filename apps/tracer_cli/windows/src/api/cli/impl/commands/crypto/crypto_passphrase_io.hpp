// api/cli/impl/commands/crypto/crypto_passphrase_io.hpp
#pragma once

#include <string>

namespace tracer_core::cli::impl::commands::crypto {

[[nodiscard]] auto PromptPassphraseForEncrypt() -> std::string;
[[nodiscard]] auto PromptPassphraseForDecrypt() -> std::string;

} // namespace tracer_core::cli::impl::commands::crypto
