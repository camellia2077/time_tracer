struct CryptoEncryptArgs {
  fs::path input_path;
  fs::path output_path;
  DateCheckMode date_check_mode = DateCheckMode::kNone;
  std::string passphrase;
  app_dto::TracerExchangeSecurityLevel security_level =
      app_dto::TracerExchangeSecurityLevel::kInteractive;
};

struct CryptoDecryptArgs {
  fs::path input_path;
  std::optional<fs::path> work_root_override;
  std::string passphrase;
};

struct CryptoUnpackArgs {
  fs::path input_path;
  fs::path output_path;
  std::string passphrase;
};

struct CryptoInspectArgs {
  fs::path input_path;
  std::string passphrase;
};

[[nodiscard]] auto ToLowerAscii(std::string value) -> std::string {
  std::transform(value.begin(), value.end(), value.begin(),
                 [](unsigned char ch) -> char {
                   return static_cast<char>(std::tolower(ch));
                 });
  return value;
}

[[nodiscard]] auto ParseJsonRequest(std::string_view request_json) -> json {
  if (request_json.empty()) {
    throw std::invalid_argument("request_json must not be empty.");
  }
  try {
    return json::parse(request_json);
  } catch (const std::exception& error) {
    throw std::invalid_argument(std::string("Invalid request JSON: ") +
                                error.what());
  }
}

[[nodiscard]] auto RequireStringField(const json& payload, std::string_view key)
    -> std::string {
  const std::string key_text(key);
  const auto it = payload.find(key_text);
  if (it == payload.end() || !it->is_string() ||
      it->get_ref<const std::string&>().empty()) {
    throw std::invalid_argument("field `" + key_text +
                                "` must be a non-empty string.");
  }
  return it->get<std::string>();
}

[[nodiscard]] auto OptionalStringField(const json& payload,
                                       std::string_view key)
    -> std::optional<std::string> {
  const std::string key_text(key);
  const auto it = payload.find(key_text);
  if (it == payload.end() || it->is_null()) {
    return std::nullopt;
  }
  if (!it->is_string()) {
    throw std::invalid_argument("field `" + key_text + "` must be a string.");
  }
  return it->get<std::string>();
}

auto RejectFieldIfPresent(const json& payload, std::string_view key,
                          std::string_view message) -> void {
  if (payload.contains(std::string(key))) {
    throw std::invalid_argument(std::string(message));
  }
}

[[nodiscard]] auto ParseSecurityLevel(const std::optional<std::string>& token)
    -> app_dto::TracerExchangeSecurityLevel {
  const std::string normalized = ToLowerAscii(token.value_or("interactive"));
  if (normalized == "min") {
    return app_dto::TracerExchangeSecurityLevel::kMin;
  }
  if (normalized == "interactive") {
    return app_dto::TracerExchangeSecurityLevel::kInteractive;
  }
  if (normalized == "moderate") {
    return app_dto::TracerExchangeSecurityLevel::kModerate;
  }
  if (normalized == "high" || normalized == "sensitive") {
    return app_dto::TracerExchangeSecurityLevel::kHigh;
  }
  if (normalized == "max") {
    return app_dto::TracerExchangeSecurityLevel::kMax;
  }
  throw std::invalid_argument(
      "field `security_level` must be one of: "
      "min|interactive|moderate|high|max (alias: sensitive).");
}

[[nodiscard]] auto ParseCryptoEncryptArgs(std::string_view request_json)
    -> CryptoEncryptArgs {
  const json payload = ParseJsonRequest(request_json);

  DateCheckMode date_check_mode = DateCheckMode::kNone;
  if (const auto mode = OptionalStringField(payload, "date_check_mode");
      mode.has_value()) {
    date_check_mode = ParseDateCheckMode(*mode);
  }

  return {
      .input_path =
          fs::absolute(fs::path(RequireStringField(payload, "input_path"))),
      .output_path =
          fs::absolute(fs::path(RequireStringField(payload, "output_path"))),
      .date_check_mode = date_check_mode,
      .passphrase = RequireStringField(payload, "passphrase"),
      .security_level =
          ParseSecurityLevel(OptionalStringField(payload, "security_level")),
  };
}

[[nodiscard]] auto ParseCryptoDecryptArgs(std::string_view request_json)
    -> CryptoDecryptArgs {
  const json payload = ParseJsonRequest(request_json);
  const auto output_path = OptionalStringField(payload, "output_path");
  RejectFieldIfPresent(payload, "apply_config",
                       "field `apply_config` is no longer supported for "
                       "tracer exchange import.");
  if (output_path.has_value() && output_path->empty()) {
    throw std::invalid_argument(
        "field `output_path` must be a non-empty string when provided.");
  }

  return {
      .input_path =
          fs::absolute(fs::path(RequireStringField(payload, "input_path"))),
      .work_root_override =
          output_path.has_value()
              ? std::optional<fs::path>(fs::absolute(fs::path(*output_path)))
              : std::nullopt,
      .passphrase = RequireStringField(payload, "passphrase"),
  };
}

[[nodiscard]] auto ParseCryptoInspectArgs(std::string_view request_json)
    -> CryptoInspectArgs {
  const json payload = ParseJsonRequest(request_json);
  RejectFieldIfPresent(payload, "output_path",
                       "field `output_path` is not supported for crypto "
                       "inspect.");

  return {
      .input_path =
          fs::absolute(fs::path(RequireStringField(payload, "input_path"))),
      .passphrase = RequireStringField(payload, "passphrase"),
  };
}

[[nodiscard]] auto ParseCryptoUnpackArgs(std::string_view request_json)
    -> CryptoUnpackArgs {
  const json payload = ParseJsonRequest(request_json);

  return {
      .input_path =
          fs::absolute(fs::path(RequireStringField(payload, "input_path"))),
      .output_path =
          fs::absolute(fs::path(RequireStringField(payload, "output_path"))),
      .passphrase = RequireStringField(payload, "passphrase"),
  };
}
