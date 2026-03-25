auto ToLowerAscii(std::string value) -> std::string {
  std::transform(value.begin(), value.end(), value.begin(),
                 [](unsigned char ch) -> char {
                   return static_cast<char>(std::tolower(ch));
                 });
  return value;
}

auto HasExtensionCaseInsensitive(const fs::path& path,
                                 std::string_view ext_lower) -> bool {
  return ToLowerAscii(path.extension().string()) == ext_lower;
}

auto ParseMonthInfoFromFileName(std::string_view file_name,
                                std::string_view source_label)
    -> ParsedMonthInfo {
  if (file_name.size() != 11U || file_name[4] != '-' ||
      file_name.substr(7U) != ".txt") {
    throw std::runtime_error(
        "Expected canonical month TXT file name YYYY-MM.txt: " +
        std::string(source_label));
  }
  for (const std::size_t index :
       {std::size_t{0}, std::size_t{1}, std::size_t{2}, std::size_t{3},
        std::size_t{5}, std::size_t{6}}) {
    const char ch = file_name[index];
    if (ch < '0' || ch > '9') {
      throw std::runtime_error(
          "Expected canonical month TXT file name YYYY-MM.txt: " +
          std::string(source_label));
    }
  }

  const int year = std::stoi(std::string(file_name.substr(0U, 4U)));
  const int month = std::stoi(std::string(file_name.substr(5U, 2U)));
  if (month < 1 || month > 12) {
    throw std::runtime_error("Invalid month file name: " +
                             std::string(source_label));
  }

  ParsedMonthInfo result{};
  result.year = year;
  result.month = month;
  result.month_key = std::string(file_name.substr(0U, 7U));
  result.file_name = std::string(file_name);
  return result;
}

auto ParseMonthInfoFromCanonicalText(std::span<const std::uint8_t> bytes,
                                     std::string_view source_label)
    -> ParsedMonthInfo {
  const std::string text = modtext::RequireCanonicalText(bytes, source_label);
  std::optional<int> year;
  std::optional<int> month;
  std::istringstream stream(text);
  std::string line;
  while (std::getline(stream, line)) {
    if (!line.empty() && line.back() == '\r') {
      line.pop_back();
    }
    const std::string_view trimmed(line);
    if (trimmed.empty()) {
      continue;
    }
    if (!year.has_value() && trimmed.size() == 5U && trimmed.front() == 'y' &&
        std::all_of(trimmed.begin() + 1U, trimmed.end(),
                    [](char ch) { return ch >= '0' && ch <= '9'; })) {
      year = std::stoi(std::string(trimmed.substr(1U)));
      continue;
    }
    if (year.has_value() && !month.has_value() && trimmed.size() == 3U &&
        trimmed.front() == 'm' &&
        std::all_of(trimmed.begin() + 1U, trimmed.end(),
                    [](char ch) { return ch >= '0' && ch <= '9'; })) {
      month = std::stoi(std::string(trimmed.substr(1U)));
      if (*month < 1 || *month > 12) {
        throw std::runtime_error("Invalid month header in " +
                                 std::string(source_label));
      }
      break;
    }
  }

  if (!year.has_value() || !month.has_value()) {
    throw std::runtime_error(
        "TXT payload requires canonical month headers yYYYY + mMM: " +
        std::string(source_label));
  }

  ParsedMonthInfo result{};
  result.year = *year;
  result.month = *month;
  std::ostringstream month_key_stream;
  month_key_stream << std::setw(4) << std::setfill('0') << result.year << "-"
                   << std::setw(2) << std::setfill('0') << result.month;
  result.month_key = month_key_stream.str();
  result.file_name = result.month_key + ".txt";
  return result;
}

auto SanitizeStem(std::string value) -> std::string {
  if (value.empty()) {
    return "tracer_exchange";
  }
  for (char& ch : value) {
    const unsigned char raw = static_cast<unsigned char>(ch);
    const bool keep =
        std::isalnum(raw) != 0 || ch == '.' || ch == '-' || ch == '_';
    if (!keep) {
      ch = '_';
    }
  }
  return value;
}

auto BuildUniqueSuffix() -> std::string {
  const auto now = std::chrono::system_clock::now().time_since_epoch();
  const auto micros =
      std::chrono::duration_cast<std::chrono::microseconds>(now).count();
  return std::to_string(micros);
}

auto BuildScopedStagingDir(const fs::path& writable_root,
                           std::string_view purpose, std::string_view stem)
    -> fs::path {
  const fs::path base_root =
      writable_root.empty() ? fs::current_path() : fs::absolute(writable_root);
  return base_root / ".tracer_staging" / std::string(purpose) /
         (SanitizeStem(std::string(stem)) + "-" + BuildUniqueSuffix());
}

auto EnsureDirectoryRemoved(const fs::path& path) -> void {
  std::error_code error;
  fs::remove_all(path, error);
  if (error) {
    throw std::runtime_error("Failed to remove directory: " + path.string() +
                             " | " + error.message());
  }
}

auto RemoveDirectoryBestEffort(const fs::path& path) -> void {
  std::error_code error;
  fs::remove_all(path, error);
}

auto EnsureParentDirectory(const fs::path& path) -> void {
  const fs::path parent = path.parent_path();
  if (parent.empty()) {
    return;
  }
  std::error_code error;
  fs::create_directories(parent, error);
  if (error) {
    throw std::runtime_error("Failed to create parent directory: " +
                             parent.string() + " | " + error.message());
  }
}

auto ReadFileBytes(const fs::path& path) -> std::vector<std::uint8_t> {
  std::ifstream input(path, std::ios::binary);
  if (!input.is_open()) {
    throw std::runtime_error("Failed to open file for reading: " +
                             path.string());
  }
  return {std::istreambuf_iterator<char>(input),
          std::istreambuf_iterator<char>()};
}

auto WriteFileBytes(const fs::path& path, std::span<const std::uint8_t> bytes)
    -> void {
  EnsureParentDirectory(path);
  std::ofstream output(path, std::ios::binary | std::ios::trunc);
  if (!output.is_open()) {
    throw std::runtime_error("Failed to open file for writing: " +
                             path.string());
  }
  if (!bytes.empty()) {
    output.write(reinterpret_cast<const char*>(bytes.data()),
                 static_cast<std::streamsize>(bytes.size()));
  }
  if (!output.good()) {
    throw std::runtime_error("Failed to write file: " + path.string());
  }
}

auto IsCanonicalTextPackagePath(std::string_view relative_path) -> bool {
  if (relative_path == exchange_pkg::kManifestPath) {
    return true;
  }
  const std::string extension =
      ToLowerAscii(fs::path(relative_path).extension().string());
  return extension == ".txt" || extension == ".toml";
}

auto CanonicalizePackageTextBytes(std::span<const std::uint8_t> bytes,
                                  std::string_view source_label)
    -> std::vector<std::uint8_t> {
  return modtext::ToUtf8Bytes(
      modtext::RequireCanonicalText(bytes, source_label));
}

auto CanonicalizePackageTextBytes(std::string_view text,
                                  std::string_view source_label)
    -> std::vector<std::uint8_t> {
  return modtext::ToUtf8Bytes(
      modtext::RequireCanonicalText(text, source_label));
}

auto EnsureRegularFileExists(const fs::path& path, std::string_view label)
    -> void {
  if (!fs::exists(path) || !fs::is_regular_file(path)) {
    throw std::runtime_error(std::string(label) +
                             " is missing: " + path.string());
  }
}

auto EnsureCryptoResultOk(const file_crypto::FileCryptoResult& result,
                          std::string_view action, const fs::path& input_path)
    -> void {
  if (result.ok()) {
    return;
  }
  throw std::runtime_error(std::string(action) + " failed for " +
                           input_path.string() + ": " + result.error_code +
                           " | " + result.error_message);
}

auto WriteDecodedPackageToRoot(
    const exchange_pkg::DecodedTracerExchangePackage& package,
    const fs::path& root) -> void {
  for (const auto& entry : package.entries) {
    const fs::path target = root / fs::path(entry.relative_path);
    if (IsCanonicalTextPackagePath(entry.relative_path)) {
      const auto canonical_bytes =
          CanonicalizePackageTextBytes(entry.data, target.string());
      WriteFileBytes(target, canonical_bytes);
      continue;
    }
    WriteFileBytes(target, entry.data);
  }
}
