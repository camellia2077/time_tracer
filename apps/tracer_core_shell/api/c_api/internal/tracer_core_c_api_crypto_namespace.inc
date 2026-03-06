namespace {

namespace fs = std::filesystem;
namespace file_crypto = tracer_core::infrastructure::crypto;
using nlohmann::json;

[[nodiscard]] auto ToLowerAscii(std::string value) -> std::string {
  std::ranges::transform(value, value.begin(), [](unsigned char ch) -> char {
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
  const auto key_text = std::string(key);
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
  const auto key_text = std::string(key);
  const auto it = payload.find(key_text);
  if (it == payload.end() || it->is_null()) {
    return std::nullopt;
  }
  if (!it->is_string()) {
    throw std::invalid_argument("field `" + key_text + "` must be a string.");
  }
  return it->get<std::string>();
}

[[nodiscard]] auto ParseSecurityLevel(const std::optional<std::string>& token)
    -> file_crypto::FileCryptoSecurityLevel {
  const std::string normalized = ToLowerAscii(token.value_or("interactive"));
  if (normalized == "min") {
    return file_crypto::FileCryptoSecurityLevel::kMin;
  }
  if (normalized == "interactive") {
    return file_crypto::FileCryptoSecurityLevel::kInteractive;
  }
  if (normalized == "moderate") {
    return file_crypto::FileCryptoSecurityLevel::kModerate;
  }
  if (normalized == "high" || normalized == "sensitive") {
    return file_crypto::FileCryptoSecurityLevel::kHigh;
  }
  if (normalized == "max") {
    return file_crypto::FileCryptoSecurityLevel::kMax;
  }
  throw std::invalid_argument(
      "field `security_level` must be one of: "
      "min|interactive|moderate|high|max (alias: sensitive).");
}

[[nodiscard]] auto HasExtensionCaseInsensitive(const fs::path& path,
                                               std::string_view ext_lower)
    -> bool {
  return ToLowerAscii(path.extension().string()) == ext_lower;
}

[[nodiscard]] auto ResolveFileOutputPath(const fs::path& input_file,
                                         const fs::path& output_arg,
                                         std::string_view extension_lower)
    -> fs::path {
  fs::path output_path = output_arg;
  if (fs::exists(output_path) && fs::is_directory(output_path)) {
    output_path /= input_file.filename();
  }
  output_path.replace_extension(extension_lower);
  return output_path;
}

void EnsureValidationSuccess(const tracer_core::core::dto::OperationAck& ack,
                             std::string_view fallback_message) {
  if (ack.ok) {
    return;
  }
  if (!ack.error_message.empty()) {
    throw std::runtime_error(ack.error_message);
  }
  throw std::runtime_error(std::string(fallback_message));
}

void ValidateInputForEncryption(ITracerCoreApi& runtime, DateCheckMode mode,
                                const fs::path& input_path) {
  const std::string input = input_path.string();
  EnsureValidationSuccess(runtime.RunValidateStructure(
                              ValidateStructureRequest{.input_path = input}),
                          "TXT structure validation failed before encryption.");

  EnsureValidationSuccess(runtime.RunValidateLogic(ValidateLogicRequest{
                              .input_path = input, .date_check_mode = mode}),
                          "TXT logic validation failed before encryption.");
}

void EnsureCryptoResultOk(const file_crypto::FileCryptoResult& result,
                          std::string_view action, const fs::path& input_path) {
  if (result.ok()) {
    return;
  }
  throw std::runtime_error(std::string(action) + " failed for " +
                           input_path.string() + ": " + result.error_code +
                           " | " + result.error_message);
}

auto EmitCryptoProgress(
    const file_crypto::FileCryptoProgressSnapshot& snapshot)
    -> file_crypto::FileCryptoControl {
  const auto registration =
      tracer_core::core::c_api::internal::GetCryptoProgressCallbackRegistration();
  if (registration.callback == nullptr) {
    return file_crypto::FileCryptoControl::kContinue;
  }

  const std::string progress_json =
      tracer_core::api::shared::crypto_progress::BuildProgressSnapshotJson(
          snapshot);
  try {
    registration.callback(progress_json.c_str(), registration.user_data);
  } catch (...) {
    // Host callback failures must not interrupt core execution.
  }
  return file_crypto::FileCryptoControl::kContinue;
}

[[nodiscard]] auto BuildBatchFailureMessage(
    std::string_view action, const file_crypto::FileCryptoBatchResult& batch)
    -> std::string {
  std::ostringstream message;
  message << action << " batch failed";
  if (!batch.status.error_code.empty() || !batch.status.error_message.empty()) {
    message << ": " << batch.status.error_code << " | "
            << batch.status.error_message;
  }
  if (batch.cancelled ||
      batch.status.error == file_crypto::FileCryptoError::kCancelled) {
    message << " (cancelled)";
  }
  if (!batch.file_errors.empty()) {
    const auto& first_error = batch.file_errors.front();
    message << " | file_errors=" << batch.file_errors.size()
            << " | first=" << first_error.input_path.string() << " => "
            << first_error.error_code << " | " << first_error.error_message;
  }
  return message.str();
}

[[nodiscard]] auto CollectFilesByExtension(const fs::path& root,
                                           std::string_view ext_lower)
    -> std::vector<fs::path> {
  if (!fs::exists(root)) {
    throw std::invalid_argument("Input path does not exist: " + root.string());
  }
  if (!fs::is_directory(root)) {
    throw std::invalid_argument("Expected directory path: " + root.string());
  }

  std::vector<fs::path> files;
  for (const auto& entry : fs::recursive_directory_iterator(root)) {
    if (!entry.is_regular_file()) {
      continue;
    }
    if (HasExtensionCaseInsensitive(entry.path(), ext_lower)) {
      files.push_back(entry.path());
    }
  }
  std::ranges::sort(files);
  return files;
}

[[nodiscard]] auto BuildInspectContent(const fs::path& file_path)
    -> std::string {
  file_crypto::TracerFileMetadata metadata{};
  const auto result = file_crypto::InspectEncryptedFile(file_path, &metadata);
  EnsureCryptoResultOk(result, "Inspect", file_path);

  std::ostringstream stream;
  stream << "File: " << file_path.string() << '\n';
  stream << "  version: " << static_cast<int>(metadata.version) << '\n';
  stream << "  kdf_id: " << static_cast<int>(metadata.kdf_id) << '\n';
  stream << "  cipher_id: " << static_cast<int>(metadata.cipher_id) << '\n';
  stream << "  compression_id: " << static_cast<int>(metadata.compression_id)
         << '\n';
  stream << "  compression_level: "
         << static_cast<int>(metadata.compression_level) << '\n';
  stream << "  ops_limit: " << metadata.ops_limit << '\n';
  stream << "  mem_limit_kib: " << metadata.mem_limit_kib << '\n';
  stream << "  plaintext_size: " << metadata.plaintext_size << '\n';
  stream << "  ciphertext_size: " << metadata.ciphertext_size << '\n';
  return stream.str();
}

[[nodiscard]] auto RunCryptoEncrypt(ITracerCoreApi& runtime,
                                    std::string_view request_json)
    -> TextOutput {
  const json payload = ParseJsonRequest(request_json);

  const fs::path input_path =
      fs::absolute(fs::path(RequireStringField(payload, "input_path")));
  const fs::path output_path =
      fs::absolute(fs::path(RequireStringField(payload, "output_path")));
  const std::string passphrase = RequireStringField(payload, "passphrase");

  DateCheckMode date_check_mode = DateCheckMode::kNone;
  if (const auto mode = OptionalStringField(payload, "date_check_mode");
      mode.has_value()) {
    date_check_mode = ParseDateCheckMode(*mode);
  }

  file_crypto::FileCryptoOptions options{};
  options.security_level =
      ParseSecurityLevel(OptionalStringField(payload, "security_level"));
  options.progress_callback =
      [](const file_crypto::FileCryptoProgressSnapshot& snapshot) {
        return EmitCryptoProgress(snapshot);
      };

  if (!fs::exists(input_path)) {
    throw std::invalid_argument("Input path does not exist: " +
                                input_path.string());
  }

  if (fs::is_regular_file(input_path)) {
    if (!HasExtensionCaseInsensitive(input_path, ".txt")) {
      throw std::invalid_argument("Encrypt input file must be .txt: " +
                                  input_path.string());
    }
    ValidateInputForEncryption(runtime, date_check_mode, input_path);
    const fs::path resolved_output =
        ResolveFileOutputPath(input_path, output_path, ".tracer");
    EnsureCryptoResultOk(file_crypto::EncryptFile(input_path, resolved_output,
                                                  passphrase, options),
                         "Encrypt", input_path);
    return TextOutput{
        .ok = true,
        .content = "Encrypted: " + input_path.string() + " -> " +
                   resolved_output.string(),
        .error_message = "",
    };
  }

  if (!fs::is_directory(input_path)) {
    throw std::invalid_argument(
        "Encrypt input path must be file or directory.");
  }
  if (fs::exists(output_path) && fs::is_regular_file(output_path)) {
    throw std::invalid_argument(
        "Encrypt output path must be a directory when input is directory: " +
        output_path.string());
  }

  ValidateInputForEncryption(runtime, date_check_mode, input_path);
  const auto batch = file_crypto::EncryptDirectory(input_path, output_path,
                                                   passphrase, options);
  if (!batch.ok()) {
    throw std::runtime_error(BuildBatchFailureMessage("Encrypt", batch));
  }
  return TextOutput{
      .ok = true,
      .content = "Encrypted " + std::to_string(batch.succeeded_files) +
                 " txt files to: " + output_path.string(),
      .error_message = "",
  };
}

[[nodiscard]] auto RunCryptoDecrypt(std::string_view request_json)
    -> TextOutput {
  const json payload = ParseJsonRequest(request_json);

  const fs::path input_path =
      fs::absolute(fs::path(RequireStringField(payload, "input_path")));
  const fs::path output_path =
      fs::absolute(fs::path(RequireStringField(payload, "output_path")));
  const std::string passphrase = RequireStringField(payload, "passphrase");
  file_crypto::FileCryptoOptions options{};
  options.progress_callback =
      [](const file_crypto::FileCryptoProgressSnapshot& snapshot) {
        return EmitCryptoProgress(snapshot);
      };

  if (!fs::exists(input_path)) {
    throw std::invalid_argument("Input path does not exist: " +
                                input_path.string());
  }

  if (fs::is_regular_file(input_path)) {
    if (!HasExtensionCaseInsensitive(input_path, ".tracer")) {
      throw std::invalid_argument("Decrypt input file must be .tracer: " +
                                  input_path.string());
    }
    const fs::path resolved_output =
        ResolveFileOutputPath(input_path, output_path, ".txt");
    EnsureCryptoResultOk(file_crypto::DecryptFile(input_path, resolved_output,
                                                  passphrase, options),
                         "Decrypt", input_path);
    return TextOutput{
        .ok = true,
        .content = "Decrypted: " + input_path.string() + " -> " +
                   resolved_output.string(),
        .error_message = "",
    };
  }

  if (!fs::is_directory(input_path)) {
    throw std::invalid_argument(
        "Decrypt input path must be file or directory.");
  }
  if (fs::exists(output_path) && fs::is_regular_file(output_path)) {
    throw std::invalid_argument(
        "Decrypt output path must be a directory when input is directory: " +
        output_path.string());
  }

  const auto batch = file_crypto::DecryptDirectory(input_path, output_path,
                                                   passphrase, options);
  if (!batch.ok()) {
    throw std::runtime_error(BuildBatchFailureMessage("Decrypt", batch));
  }
  return TextOutput{
      .ok = true,
      .content = "Decrypted " + std::to_string(batch.succeeded_files) +
                 " tracer files to: " + output_path.string(),
      .error_message = "",
  };
}

[[nodiscard]] auto RunCryptoInspect(std::string_view request_json)
    -> TextOutput {
  const json payload = ParseJsonRequest(request_json);
  const fs::path input_path =
      fs::absolute(fs::path(RequireStringField(payload, "input_path")));

  if (!fs::exists(input_path)) {
    throw std::invalid_argument("Input path does not exist: " +
                                input_path.string());
  }

  if (fs::is_regular_file(input_path)) {
    return TextOutput{
        .ok = true,
        .content = BuildInspectContent(input_path),
        .error_message = "",
    };
  }
  if (!fs::is_directory(input_path)) {
    throw std::invalid_argument(
        "Inspect input path must be file or directory.");
  }

  const auto tracer_files = CollectFilesByExtension(input_path, ".tracer");
  if (tracer_files.empty()) {
    throw std::runtime_error("No .tracer files found under directory: " +
                             input_path.string());
  }

  std::ostringstream output;
  for (size_t index = 0; index < tracer_files.size(); ++index) {
    if (index > 0) {
      output << '\n';
    }
    output << BuildInspectContent(tracer_files[index]);
  }

  return TextOutput{
      .ok = true,
      .content = output.str(),
      .error_message = "",
  };
}

}  // namespace
