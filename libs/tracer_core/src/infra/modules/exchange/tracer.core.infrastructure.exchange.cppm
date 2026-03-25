module;

#include <algorithm>
#include <array>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <span>
#include <utility>
#include <vector>

#if defined(TT_HAS_LIBSODIUM) && TT_HAS_LIBSODIUM
#include <sodium.h>
#endif

#include <toml++/toml.h>

export module tracer.core.infrastructure.exchange;

export namespace tracer::core::infrastructure::crypto::exchange {

inline constexpr std::string_view kManifestPath = "manifest.toml";
inline constexpr std::string_view kConverterMainPath =
    "config/converter/interval_processor_config.toml";
inline constexpr std::string_view kAliasMappingPath =
    "config/converter/alias_mapping.toml";
inline constexpr std::string_view kDurationRulesPath =
    "config/converter/duration_rules.toml";
inline constexpr std::string_view kPayloadRoot = "payload";

inline constexpr std::array<std::string_view, 4> kRequiredPackagePaths = {
    kManifestPath,
    kConverterMainPath,
    kAliasMappingPath,
    kDurationRulesPath,
};

inline constexpr std::uint16_t kEntryFlagRequired = 0x0001U;
inline constexpr std::uint16_t kEntryFlagText = 0x0002U;
inline constexpr std::uint16_t kStandardEntryFlags =
    kEntryFlagRequired | kEntryFlagText;

struct TracerExchangeManifest {
  std::string package_type = "tracer_exchange";
  std::int64_t package_version = 3;
  std::string producer_platform;
  std::string producer_app;
  std::string created_at_utc;
  std::string source_root_name;
  std::string payload_root = std::string(kPayloadRoot);
  std::vector<std::string> payload_files;
  std::string converter_main_config = std::string(kConverterMainPath);
  std::string converter_alias_mapping = std::string(kAliasMappingPath);
  std::string converter_duration_rules = std::string(kDurationRulesPath);
};

struct TracerExchangePackageEntry {
  std::string relative_path;
  std::vector<std::uint8_t> data;
  std::uint16_t entry_flags = kStandardEntryFlags;
  std::array<std::uint8_t, 32> sha256{};
};

struct DecodedTracerExchangePackage {
  TracerExchangeManifest manifest;
  std::vector<TracerExchangePackageEntry> entries;
};

[[nodiscard]] auto BuildManifestText(const TracerExchangeManifest& manifest)
    -> std::string;
[[nodiscard]] auto ParseManifestText(std::string_view manifest_text)
    -> TracerExchangeManifest;

[[nodiscard]] auto EncodePackageBytes(
    const std::vector<TracerExchangePackageEntry>& entries)
    -> std::vector<std::uint8_t>;
[[nodiscard]] auto DecodePackageBytes(std::span<const std::uint8_t> bytes)
    -> DecodedTracerExchangePackage;

}  // namespace tracer::core::infrastructure::crypto::exchange

namespace tracer::core::infrastructure::crypto::exchange {

namespace detail {

struct PackageEntryRecord {
  std::string path;
  std::uint16_t flags = kStandardEntryFlags;
  std::uint64_t offset = 0U;
  std::uint64_t size = 0U;
  std::array<std::uint8_t, 32> sha256{};
  const std::vector<std::uint8_t>* data = nullptr;
};

}  // namespace detail

namespace {

constexpr std::string_view kPackageMagic = "TTPKG";
constexpr std::uint8_t kPackageVersion = 3U;
constexpr std::uint16_t kPackageHeaderSize = 32U;
constexpr std::uint16_t kPackageFlags = 0U;
constexpr std::uint16_t kManifestIndex = 0U;

constexpr std::string_view kMalformedPackagePrefix =
    "unsupported/malformed tracer package";

auto AppendBytes(std::vector<std::uint8_t>& out,
                 std::span<const std::uint8_t> in) -> void {
  out.insert(out.end(), in.begin(), in.end());
}

auto AppendU16LE(std::vector<std::uint8_t>& out, std::uint16_t value) -> void {
  out.push_back(static_cast<std::uint8_t>(value & 0xFFU));
  out.push_back(static_cast<std::uint8_t>((value >> 8U) & 0xFFU));
}

auto AppendU64LE(std::vector<std::uint8_t>& out, std::uint64_t value) -> void {
  for (std::size_t i = 0; i < sizeof(value); ++i) {
    out.push_back(static_cast<std::uint8_t>((value >> (i * 8U)) & 0xFFU));
  }
}

auto WriteU16LE(std::vector<std::uint8_t>& out, std::size_t offset,
                std::uint16_t value) -> void {
  out[offset] = static_cast<std::uint8_t>(value & 0xFFU);
  out[offset + 1U] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
}

auto WriteU32LE(std::vector<std::uint8_t>& out, std::size_t offset,
                std::uint32_t value) -> void {
  for (std::size_t i = 0; i < sizeof(value); ++i) {
    out[offset + i] = static_cast<std::uint8_t>((value >> (i * 8U)) & 0xFFU);
  }
}

auto WriteU64LE(std::vector<std::uint8_t>& out, std::size_t offset,
                std::uint64_t value) -> void {
  for (std::size_t i = 0; i < sizeof(value); ++i) {
    out[offset + i] = static_cast<std::uint8_t>((value >> (i * 8U)) & 0xFFU);
  }
}

[[nodiscard]] auto ReadU16LE(std::span<const std::uint8_t> bytes,
                             std::size_t offset) -> std::uint16_t {
  if (offset + 2U > bytes.size()) {
    throw std::runtime_error("Unexpected end of package while reading u16.");
  }
  return static_cast<std::uint16_t>(bytes[offset]) |
         (static_cast<std::uint16_t>(bytes[offset + 1U]) << 8U);
}

[[nodiscard]] auto ReadU32LE(std::span<const std::uint8_t> bytes,
                             std::size_t offset) -> std::uint32_t {
  if (offset + 4U > bytes.size()) {
    throw std::runtime_error("Unexpected end of package while reading u32.");
  }
  std::uint32_t value = 0U;
  for (std::size_t i = 0; i < 4U; ++i) {
    value |= static_cast<std::uint32_t>(bytes[offset + i]) << (i * 8U);
  }
  return value;
}

[[nodiscard]] auto ReadU64LE(std::span<const std::uint8_t> bytes,
                             std::size_t offset) -> std::uint64_t {
  if (offset + 8U > bytes.size()) {
    throw std::runtime_error("Unexpected end of package while reading u64.");
  }
  std::uint64_t value = 0U;
  for (std::size_t i = 0; i < 8U; ++i) {
    value |= static_cast<std::uint64_t>(bytes[offset + i]) << (i * 8U);
  }
  return value;
}

[[nodiscard]] auto ComputeSha256(std::span<const std::uint8_t> data)
    -> std::array<std::uint8_t, 32> {
  std::array<std::uint8_t, 32> digest{};
#if defined(TT_HAS_LIBSODIUM) && TT_HAS_LIBSODIUM
  const unsigned char* input =
      data.empty() ? nullptr
                   : reinterpret_cast<const unsigned char*>(data.data());
  if (crypto_hash_sha256(digest.data(), input,
                         static_cast<unsigned long long>(data.size())) != 0) {
    throw std::runtime_error("Failed to compute tracer exchange SHA-256.");
  }
  return digest;
#else
  (void)data;
  throw std::runtime_error(
      "Tracer exchange requires libsodium SHA-256 support.");
#endif
}

auto ThrowMalformedPackage(std::string_view detail) -> void {
  throw std::runtime_error(std::string(kMalformedPackagePrefix) + ": " +
                           std::string(detail));
}

[[nodiscard]] auto IsValidPayloadPath(std::string_view path) -> bool {
  if (path.empty() || path.find('\\') != std::string_view::npos) {
    return false;
  }
  if (!path.starts_with(kPayloadRoot) || path.size() <= kPayloadRoot.size() ||
      path[kPayloadRoot.size()] != '/') {
    return false;
  }
  if (!path.ends_with(".txt")) {
    return false;
  }

  const std::string_view relative = path.substr(kPayloadRoot.size() + 1U);
  const std::size_t slash = relative.find('/');
  if (slash == std::string_view::npos || slash == 0U ||
      slash + 1U >= relative.size()) {
    return false;
  }

  const std::string_view year_dir = relative.substr(0U, slash);
  const std::string_view filename = relative.substr(slash + 1U);
  if (year_dir.size() != 4U ||
      !std::all_of(year_dir.begin(), year_dir.end(),
                   [](char ch) { return ch >= '0' && ch <= '9'; })) {
    return false;
  }
  if (filename.size() != 11U || filename[4] != '-' ||
      filename.substr(7) != ".txt") {
    return false;
  }
  for (const std::size_t index :
       {std::size_t{0}, std::size_t{1}, std::size_t{2}, std::size_t{3},
        std::size_t{5}, std::size_t{6}}) {
    const char ch = filename[index];
    if (ch < '0' || ch > '9') {
      return false;
    }
  }
  if (filename.substr(0U, 4U) != year_dir) {
    return false;
  }
  const int month = (filename[5] - '0') * 10 + (filename[6] - '0');
  return month >= 1 && month <= 12;
}

auto ValidateManifestPayloadFiles(const std::vector<std::string>& payload_files)
    -> void {
  if (payload_files.empty()) {
    ThrowMalformedPackage(
        "manifest payload files must be a non-empty array of payload .txt "
        "paths.");
  }

  std::string previous_path;
  for (const auto& payload_path : payload_files) {
    if (!IsValidPayloadPath(payload_path)) {
      ThrowMalformedPackage(
          "manifest payload path must be under `payload/` "
          "and end with `.txt`.");
    }
    if (!previous_path.empty() && payload_path <= previous_path) {
      ThrowMalformedPackage(
          "manifest payload files must be unique and sorted "
          "lexicographically.");
    }
    previous_path = payload_path;
  }
}

auto ValidatePackageEntryLayout(
    const std::vector<TracerExchangePackageEntry>& entries,
    const TracerExchangeManifest& manifest) -> void {
  const std::size_t expected_entry_count =
      kRequiredPackagePaths.size() + manifest.payload_files.size();
  if (entries.size() != expected_entry_count) {
    ThrowMalformedPackage(
        "entry_count does not match manifest payload file "
        "count.");
  }

  for (std::size_t index = 0; index < kRequiredPackagePaths.size(); ++index) {
    if (entries[index].relative_path != kRequiredPackagePaths[index]) {
      ThrowMalformedPackage("entry order or fixed path set is invalid.");
    }
  }

  for (const auto& entry : entries) {
    if (entry.entry_flags != kStandardEntryFlags) {
      ThrowMalformedPackage("entry_flags must be required|text for all files.");
    }
  }

  for (std::size_t index = 0; index < manifest.payload_files.size(); ++index) {
    const std::string_view expected_payload_path =
        manifest.payload_files[index];
    const std::size_t entry_index = kRequiredPackagePaths.size() + index;
    if (entries[entry_index].relative_path != expected_payload_path) {
      ThrowMalformedPackage(
          "payload entry set does not match manifest "
          "payload.files.");
    }
  }
}

[[nodiscard]] auto BuildPackageBytesInternal(
    const std::vector<TracerExchangePackageEntry>& entries)
    -> std::vector<std::uint8_t> {
  std::vector<detail::PackageEntryRecord> records;
  records.reserve(entries.size());

  std::uint64_t data_offset = 0U;
  std::uint32_t toc_size_bytes = 0U;
  for (const auto& entry : entries) {
    detail::PackageEntryRecord record{};
    record.path = entry.relative_path;
    record.flags = entry.entry_flags;
    record.offset = data_offset;
    record.size = static_cast<std::uint64_t>(entry.data.size());
    record.sha256 = ComputeSha256(entry.data);
    record.data = &entry.data;
    data_offset += record.size;

    if (record.path.size() > static_cast<std::size_t>(UINT16_MAX)) {
      throw std::runtime_error("Tracer exchange path is too long: " +
                               record.path);
    }
    toc_size_bytes += static_cast<std::uint32_t>(2U + 2U + 8U + 8U + 32U +
                                                 record.path.size());
    records.push_back(std::move(record));
  }

  std::vector<std::uint8_t> bytes(kPackageHeaderSize, 0U);
  for (const auto& record : records) {
    AppendU16LE(bytes, static_cast<std::uint16_t>(record.path.size()));
    AppendU16LE(bytes, record.flags);
    AppendU64LE(bytes, record.offset);
    AppendU64LE(bytes, record.size);
    AppendBytes(bytes, record.sha256);
    AppendBytes(bytes,
                std::span<const std::uint8_t>(
                    reinterpret_cast<const std::uint8_t*>(record.path.data()),
                    record.path.size()));
  }
  for (const auto& record : records) {
    AppendBytes(bytes, *record.data);
  }

  std::copy(kPackageMagic.begin(), kPackageMagic.end(), bytes.begin());
  bytes[5U] = kPackageVersion;
  WriteU16LE(bytes, 6U, kPackageHeaderSize);
  WriteU16LE(bytes, 8U, kPackageFlags);
  if (records.size() > static_cast<std::size_t>(UINT16_MAX)) {
    throw std::runtime_error("Tracer exchange entry count exceeds UINT16_MAX.");
  }
  WriteU16LE(bytes, 10U, static_cast<std::uint16_t>(records.size()));
  WriteU16LE(bytes, 12U, kManifestIndex);
  WriteU16LE(bytes, 14U, 0U);
  WriteU32LE(bytes, 16U, toc_size_bytes);
  WriteU64LE(bytes, 20U, data_offset);
  WriteU32LE(bytes, 28U, 0U);
  return bytes;
}

[[nodiscard]] auto ParseExpectedString(const toml::table& table,
                                       std::string_view key) -> std::string {
  const auto value = table[std::string(key)].value<std::string>();
  if (!value.has_value() || value->empty()) {
    throw std::runtime_error("Manifest field `" + std::string(key) +
                             "` must be a non-empty string.");
  }
  return *value;
}

[[nodiscard]] auto ParseStringArray(const toml::table& table,
                                    std::string_view key)
    -> std::vector<std::string> {
  const toml::array* value = table[std::string(key)].as_array();
  if (value == nullptr) {
    ThrowMalformedPackage("manifest field `" + std::string(key) +
                          "` must be an array.");
  }

  std::vector<std::string> items;
  items.reserve(value->size());
  for (const auto& node : *value) {
    const auto item = node.value<std::string>();
    if (!item.has_value() || item->empty()) {
      ThrowMalformedPackage("manifest field `" + std::string(key) +
                            "` must contain only non-empty strings.");
    }
    items.push_back(*item);
  }
  return items;
}

}  // namespace

auto BuildManifestText(const TracerExchangeManifest& manifest) -> std::string {
  toml::table table;
  table.insert("package_type", manifest.package_type);
  table.insert("package_version", manifest.package_version);
  table.insert("producer_platform", manifest.producer_platform);
  table.insert("producer_app", manifest.producer_app);
  table.insert("created_at_utc", manifest.created_at_utc);
  table.insert("source_root_name", manifest.source_root_name);

  toml::table payload;
  payload.insert("root", manifest.payload_root);
  toml::array payload_files{};
  for (const auto& path : manifest.payload_files) {
    payload_files.push_back(path);
  }
  payload.insert("files", std::move(payload_files));
  table.insert("payload", std::move(payload));

  toml::table converter;
  converter.insert("main_config", manifest.converter_main_config);
  converter.insert("alias_mapping", manifest.converter_alias_mapping);
  converter.insert("duration_rules", manifest.converter_duration_rules);
  table.insert("converter", std::move(converter));

  std::ostringstream stream;
  stream << table;
  return stream.str();
}

auto ParseManifestText(std::string_view manifest_text)
    -> TracerExchangeManifest {
  toml::table table;
  try {
    table = toml::parse(std::string(manifest_text));
  } catch (const toml::parse_error& error) {
    ThrowMalformedPackage(std::string("manifest.toml parse failed: ") +
                          std::string(error.description()));
  }

  TracerExchangeManifest manifest{};
  manifest.package_type = ParseExpectedString(table, "package_type");
  manifest.producer_platform = ParseExpectedString(table, "producer_platform");
  manifest.producer_app = ParseExpectedString(table, "producer_app");
  manifest.created_at_utc = ParseExpectedString(table, "created_at_utc");
  manifest.source_root_name = ParseExpectedString(table, "source_root_name");

  const auto package_version = table["package_version"].value<std::int64_t>();
  if (!package_version.has_value()) {
    ThrowMalformedPackage("manifest.toml field `package_version` is required.");
  }
  manifest.package_version = *package_version;

  const toml::table* payload_table = table["payload"].as_table();
  if (payload_table == nullptr) {
    ThrowMalformedPackage("manifest.toml table `[payload]` is required.");
  }
  manifest.payload_root = ParseExpectedString(*payload_table, "root");
  manifest.payload_files = ParseStringArray(*payload_table, "files");

  const toml::table* converter_table = table["converter"].as_table();
  if (converter_table == nullptr) {
    ThrowMalformedPackage("manifest.toml table `[converter]` is required.");
  }
  manifest.converter_main_config =
      ParseExpectedString(*converter_table, "main_config");
  manifest.converter_alias_mapping =
      ParseExpectedString(*converter_table, "alias_mapping");
  manifest.converter_duration_rules =
      ParseExpectedString(*converter_table, "duration_rules");

  if (manifest.package_type != "tracer_exchange") {
    ThrowMalformedPackage("manifest `package_type` must be `tracer_exchange`.");
  }
  if (manifest.package_version != 3) {
    ThrowMalformedPackage("manifest `package_version` must be 3.");
  }
  if (manifest.payload_root != kPayloadRoot) {
    ThrowMalformedPackage("manifest payload root must be `payload`.");
  }
  ValidateManifestPayloadFiles(manifest.payload_files);
  if (manifest.converter_main_config != kConverterMainPath ||
      manifest.converter_alias_mapping != kAliasMappingPath ||
      manifest.converter_duration_rules != kDurationRulesPath) {
    ThrowMalformedPackage(
        "manifest converter paths must match fixed package "
        "layout.");
  }
  return manifest;
}

auto EncodePackageBytes(const std::vector<TracerExchangePackageEntry>& entries)
    -> std::vector<std::uint8_t> {
  if (entries.empty() || entries.front().relative_path != kManifestPath) {
    ThrowMalformedPackage("entry order or fixed path set is invalid.");
  }
  const TracerExchangeManifest manifest = ParseManifestText(std::string_view(
      reinterpret_cast<const char*>(entries.front().data.data()),
      entries.front().data.size()));
  ValidatePackageEntryLayout(entries, manifest);
  return BuildPackageBytesInternal(entries);
}

auto DecodePackageBytes(std::span<const std::uint8_t> bytes)
    -> DecodedTracerExchangePackage {
  if (bytes.size() < kPackageHeaderSize) {
    ThrowMalformedPackage("package header is truncated.");
  }
  if (!std::equal(
          kPackageMagic.begin(), kPackageMagic.end(), bytes.begin(),
          bytes.begin() + static_cast<std::ptrdiff_t>(kPackageMagic.size()))) {
    ThrowMalformedPackage("magic must be `TTPKG`.");
  }
  if (bytes[5U] != kPackageVersion) {
    ThrowMalformedPackage("unsupported package version.");
  }
  if (ReadU16LE(bytes, 6U) != kPackageHeaderSize) {
    ThrowMalformedPackage("header_size must be 32.");
  }
  if (ReadU16LE(bytes, 8U) != kPackageFlags) {
    ThrowMalformedPackage("package flags must be zero.");
  }

  const std::uint16_t entry_count = ReadU16LE(bytes, 10U);
  const std::uint16_t manifest_index = ReadU16LE(bytes, 12U);
  const std::uint16_t reserved_u16 = ReadU16LE(bytes, 14U);
  const std::uint32_t toc_size_bytes = ReadU32LE(bytes, 16U);
  const std::uint64_t data_section_size = ReadU64LE(bytes, 20U);
  const std::uint32_t reserved_u32 = ReadU32LE(bytes, 28U);

  if (entry_count < kRequiredPackagePaths.size() + 1U) {
    ThrowMalformedPackage(
        "entry_count must include manifest, converter files, "
        "and at least one payload text file.");
  }
  if (manifest_index != kManifestIndex) {
    ThrowMalformedPackage("manifest_index must be 0.");
  }
  if (reserved_u16 != 0U || reserved_u32 != 0U) {
    ThrowMalformedPackage("reserved header fields must be zero.");
  }

  const std::size_t data_section_start =
      static_cast<std::size_t>(kPackageHeaderSize) +
      static_cast<std::size_t>(toc_size_bytes);
  if (data_section_start > bytes.size()) {
    ThrowMalformedPackage("TOC extends beyond package size.");
  }
  if (data_section_start + static_cast<std::size_t>(data_section_size) !=
      bytes.size()) {
    ThrowMalformedPackage("data_section_size does not match package size.");
  }

  std::size_t cursor = kPackageHeaderSize;
  std::vector<TracerExchangePackageEntry> entries;
  entries.reserve(entry_count);
  for (std::size_t index = 0; index < entry_count; ++index) {
    const std::uint16_t path_len = ReadU16LE(bytes, cursor);
    cursor += 2U;
    const std::uint16_t entry_flags = ReadU16LE(bytes, cursor);
    cursor += 2U;
    const std::uint64_t data_offset = ReadU64LE(bytes, cursor);
    cursor += 8U;
    const std::uint64_t data_size = ReadU64LE(bytes, cursor);
    cursor += 8U;

    if (cursor + 32U + path_len > data_section_start) {
      ThrowMalformedPackage("TOC entry exceeds declared TOC size.");
    }

    std::array<std::uint8_t, 32> sha256{};
    std::copy(bytes.begin() + static_cast<std::ptrdiff_t>(cursor),
              bytes.begin() + static_cast<std::ptrdiff_t>(cursor + 32U),
              sha256.begin());
    cursor += 32U;

    const char* path_ptr = reinterpret_cast<const char*>(
        bytes.data() + static_cast<std::ptrdiff_t>(cursor));
    std::string relative_path(path_ptr, path_ptr + path_len);
    cursor += path_len;

    if (entry_flags != kStandardEntryFlags) {
      ThrowMalformedPackage("entry_flags must be required|text for all files.");
    }
    if (data_offset + data_size > data_section_size) {
      ThrowMalformedPackage("entry data exceeds data section bounds.");
    }

    const std::size_t absolute_offset =
        data_section_start + static_cast<std::size_t>(data_offset);
    const std::size_t absolute_end =
        absolute_offset + static_cast<std::size_t>(data_size);
    const auto computed_sha256 = ComputeSha256(std::span<const std::uint8_t>(
        bytes.data() + static_cast<std::ptrdiff_t>(absolute_offset),
        static_cast<std::size_t>(data_size)));
    if (computed_sha256 != sha256) {
      ThrowMalformedPackage("entry SHA-256 mismatch.");
    }

    TracerExchangePackageEntry entry{};
    entry.relative_path = std::move(relative_path);
    entry.entry_flags = entry_flags;
    entry.sha256 = sha256;
    entry.data.assign(
        bytes.begin() + static_cast<std::ptrdiff_t>(absolute_offset),
        bytes.begin() + static_cast<std::ptrdiff_t>(absolute_end));
    entries.push_back(std::move(entry));
  }

  if (cursor != data_section_start) {
    ThrowMalformedPackage("TOC size does not match parsed entry metadata.");
  }

  DecodedTracerExchangePackage package{};
  package.manifest = ParseManifestText(std::string_view(
      reinterpret_cast<const char*>(entries.front().data.data()),
      entries.front().data.size()));
  ValidatePackageEntryLayout(entries, package.manifest);
  package.entries = std::move(entries);
  return package;
}

}  // namespace tracer::core::infrastructure::crypto::exchange
