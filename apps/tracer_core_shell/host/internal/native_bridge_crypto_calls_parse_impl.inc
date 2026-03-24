struct EncryptFileArgs {
  fs::path input_path;
  fs::path output_path;
  std::string passphrase;
  file_crypto::FileCryptoSecurityLevel security_level =
      file_crypto::FileCryptoSecurityLevel::kInteractive;
};

struct DecryptFileArgs {
  fs::path input_path;
  fs::path output_path;
  std::string passphrase;
};

struct ExportTracerExchangeArgs {
  fs::path input_path;
  fs::path output_path;
  std::string passphrase;
  app_dto::TracerExchangeSecurityLevel security_level =
      app_dto::TracerExchangeSecurityLevel::kInteractive;
  DateCheckMode date_check_mode;
};

struct ImportTracerExchangeArgs {
  fs::path input_path;
  fs::path work_root;
  std::string passphrase;
};

struct InspectTracerExchangeArgs {
  fs::path input_path;
  std::string passphrase;
};

auto RequireNonEmptyText(std::string value, std::string_view field_name)
    -> std::string {
  if (value.empty()) {
    throw std::invalid_argument(std::string(field_name) +
                                " must not be empty.");
  }
  return value;
}

auto ParseTracerExchangeSecurityLevel(std::string_view value)
    -> std::optional<app_dto::TracerExchangeSecurityLevel> {
  const auto parsed_crypto_level = ParseCryptoSecurityLevel(value);
  if (!parsed_crypto_level.has_value()) {
    return std::nullopt;
  }

  switch (parsed_crypto_level.value()) {
    case file_crypto::FileCryptoSecurityLevel::kInteractive:
      return app_dto::TracerExchangeSecurityLevel::kInteractive;
    case file_crypto::FileCryptoSecurityLevel::kModerate:
      return app_dto::TracerExchangeSecurityLevel::kModerate;
    case file_crypto::FileCryptoSecurityLevel::kHigh:
      return app_dto::TracerExchangeSecurityLevel::kHigh;
    case file_crypto::FileCryptoSecurityLevel::kMin:
      return app_dto::TracerExchangeSecurityLevel::kMin;
    case file_crypto::FileCryptoSecurityLevel::kMax:
      return app_dto::TracerExchangeSecurityLevel::kMax;
  }

  return std::nullopt;
}

auto RequireCryptoSecurityLevel(std::string_view value)
    -> file_crypto::FileCryptoSecurityLevel {
  const auto parsed_security_level = ParseCryptoSecurityLevel(value);
  if (!parsed_security_level.has_value()) {
    throw std::invalid_argument(
        "securityLevel must be one of: interactive, moderate, high.");
  }
  return parsed_security_level.value();
}

auto RequireTracerExchangeSecurityLevel(std::string_view value)
    -> app_dto::TracerExchangeSecurityLevel {
  const auto parsed_security_level = ParseTracerExchangeSecurityLevel(value);
  if (!parsed_security_level.has_value()) {
    throw std::invalid_argument(
        "securityLevel must be one of: interactive, moderate, high.");
  }
  return parsed_security_level.value();
}

auto ParseAndroidDateCheckMode(jint value) -> DateCheckMode {
  return tracer_core::shell::c_api_bridge::ParseDateCheckMode(
      ParseDateCheckMode(value));
}

auto ParseEncryptFileArgs(JNIEnv* env, jstring input_path, jstring output_path,
                          jstring passphrase, jstring security_level)
    -> EncryptFileArgs {
  const std::string input_path_utf8 =
      RequireNonEmptyText(ToUtf8(env, input_path), "inputPath");
  const std::string output_path_utf8 =
      RequireNonEmptyText(ToUtf8(env, output_path), "outputPath");
  const std::string passphrase_utf8 =
      RequireNonEmptyText(ToUtf8(env, passphrase), "passphrase");
  const std::string security_level_utf8 = ToUtf8(env, security_level);

  return {
      .input_path = fs::path(input_path_utf8),
      .output_path = fs::path(output_path_utf8),
      .passphrase = passphrase_utf8,
      .security_level = RequireCryptoSecurityLevel(security_level_utf8),
  };
}

auto ParseDecryptFileArgs(JNIEnv* env, jstring input_path, jstring output_path,
                          jstring passphrase) -> DecryptFileArgs {
  const std::string input_path_utf8 =
      RequireNonEmptyText(ToUtf8(env, input_path), "inputPath");
  const std::string output_path_utf8 =
      RequireNonEmptyText(ToUtf8(env, output_path), "outputPath");
  const std::string passphrase_utf8 =
      RequireNonEmptyText(ToUtf8(env, passphrase), "passphrase");

  return {
      .input_path = fs::path(input_path_utf8),
      .output_path = fs::path(output_path_utf8),
      .passphrase = passphrase_utf8,
  };
}

auto ParseExportTracerExchangeArgs(JNIEnv* env, jstring input_path,
                                   jstring output_path, jstring passphrase,
                                   jstring security_level,
                                   jint date_check_mode)
    -> ExportTracerExchangeArgs {
  const std::string input_path_utf8 =
      RequireNonEmptyText(ToUtf8(env, input_path), "inputPath");
  const std::string output_path_utf8 =
      RequireNonEmptyText(ToUtf8(env, output_path), "outputPath");
  const std::string passphrase_utf8 =
      RequireNonEmptyText(ToUtf8(env, passphrase), "passphrase");
  const std::string security_level_utf8 = ToUtf8(env, security_level);

  return {
      .input_path = fs::path(input_path_utf8),
      .output_path = fs::path(output_path_utf8),
      .passphrase = passphrase_utf8,
      .security_level =
          RequireTracerExchangeSecurityLevel(security_level_utf8),
      .date_check_mode = ParseAndroidDateCheckMode(date_check_mode),
  };
}

auto ParseImportTracerExchangeArgs(JNIEnv* env, jstring input_path,
                                   jstring work_root, jstring passphrase)
    -> ImportTracerExchangeArgs {
  const std::string input_path_utf8 =
      RequireNonEmptyText(ToUtf8(env, input_path), "inputPath");
  const std::string work_root_utf8 =
      RequireNonEmptyText(ToUtf8(env, work_root), "workRoot");
  const std::string passphrase_utf8 =
      RequireNonEmptyText(ToUtf8(env, passphrase), "passphrase");

  return {
      .input_path = fs::path(input_path_utf8),
      .work_root = fs::path(work_root_utf8),
      .passphrase = passphrase_utf8,
  };
}

auto ParseInspectTracerExchangeArgs(JNIEnv* env, jstring input_path,
                                    jstring passphrase)
    -> InspectTracerExchangeArgs {
  const std::string input_path_utf8 =
      RequireNonEmptyText(ToUtf8(env, input_path), "inputPath");
  const std::string passphrase_utf8 =
      RequireNonEmptyText(ToUtf8(env, passphrase), "passphrase");

  return {
      .input_path = fs::path(input_path_utf8),
      .passphrase = passphrase_utf8,
  };
}
