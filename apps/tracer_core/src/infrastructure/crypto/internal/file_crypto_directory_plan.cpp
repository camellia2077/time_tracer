// infrastructure/crypto/internal/file_crypto_directory_plan.cpp
#include "infrastructure/crypto/internal/file_crypto_directory_plan.hpp"

#include <algorithm>
#include <cctype>
#include <map>
#include <system_error>

#include "infrastructure/crypto/internal/file_crypto_common.hpp"

namespace tracer_core::infrastructure::crypto::internal {

namespace {

[[nodiscard]] auto ToLowerAscii(std::string value) -> std::string {
  std::transform(value.begin(), value.end(), value.begin(),
                 [](unsigned char ch) -> char {
                   return static_cast<char>(std::tolower(ch));
                 });
  return value;
}

[[nodiscard]] auto HasExtensionCaseInsensitive(const fs::path& path,
                                               std::string_view ext_lower)
    -> bool {
  return ToLowerAscii(path.extension().string()) == ext_lower;
}

[[nodiscard]] auto ResolveTopLevelGroupLabel(const fs::path& root,
                                             const fs::path& file_path)
    -> std::string {
  std::error_code relative_error;
  const fs::path relative = fs::relative(file_path, root, relative_error);
  if (relative_error) {
    return "(root)";
  }
  const fs::path parent = relative.parent_path();
  if (parent.empty() || parent == ".") {
    return "(root)";
  }
  const auto it = parent.begin();
  if (it == parent.end()) {
    return "(root)";
  }
  const std::string label = it->string();
  return label.empty() ? "(root)" : label;
}

}  // namespace

auto BuildSingleFilePlanEntry(const fs::path& input_path,
                              const fs::path& output_path)
    -> std::pair<FileCryptoResult, DirectoryTaskPlanEntry> {
  std::error_code size_error;
  const auto input_size = fs::file_size(input_path, size_error);
  if (size_error) {
    return {MakeError(FileCryptoError::kInputReadFailed,
                      "Failed to read input file size."),
            {}};
  }

  DirectoryTaskPlanEntry entry{};
  entry.input_path = input_path;
  entry.output_path = output_path;
  entry.group_label = "(root)";
  entry.input_size_bytes = input_size;
  entry.group_index = 1;
  entry.group_file_index = 1;
  entry.group_file_count = 1;
  entry.file_index = 1;
  entry.total_files = 1;
  return {{}, std::move(entry)};
}

auto BuildDirectoryTaskPlan(const fs::path& input_root_path,
                            const fs::path& output_root_path,
                            std::string_view input_extension_lower,
                            std::string_view output_extension_lower)
    -> std::pair<FileCryptoResult, DirectoryTaskPlan> {
  if (input_root_path.empty() || output_root_path.empty()) {
    return {MakeError(FileCryptoError::kInvalidArgument,
                      "Input and output paths are required."),
            {}};
  }
  if (!fs::exists(input_root_path)) {
    return {MakeError(FileCryptoError::kInvalidArgument,
                      "Input directory does not exist."),
            {}};
  }
  if (!fs::is_directory(input_root_path)) {
    return {MakeError(FileCryptoError::kInvalidArgument,
                      "Input path must be a directory."),
            {}};
  }
  if (fs::exists(output_root_path) && fs::is_regular_file(output_root_path)) {
    return {
        MakeError(FileCryptoError::kInvalidArgument,
                  "Output path must be a directory when input is directory."),
        {}};
  }

  std::vector<fs::path> files;
  std::uint64_t total_input_bytes = 0;
  for (const auto& entry : fs::recursive_directory_iterator(input_root_path)) {
    if (!entry.is_regular_file()) {
      continue;
    }
    const fs::path file_path = entry.path();
    if (!HasExtensionCaseInsensitive(file_path, input_extension_lower)) {
      continue;
    }

    std::error_code size_error;
    const auto file_size = fs::file_size(file_path, size_error);
    if (size_error) {
      return {MakeError(FileCryptoError::kInputReadFailed,
                        "Failed to read input file size during scan."),
              {}};
    }

    files.push_back(file_path);
    total_input_bytes += file_size;
  }

  if (files.empty()) {
    return {
        MakeError(
            FileCryptoError::kInvalidArgument,
            "No matching files found under input directory for extension: " +
                std::string(input_extension_lower)),
        {}};
  }

  std::sort(files.begin(), files.end());

  std::map<std::string, std::size_t> group_file_counts;
  for (const auto& file_path : files) {
    const auto kGroupLabel =
        ResolveTopLevelGroupLabel(input_root_path, file_path);
    ++group_file_counts[kGroupLabel];
  }

  DirectoryTaskPlan plan{};
  plan.total_input_bytes = total_input_bytes;
  plan.group_count = group_file_counts.size();
  plan.entries.reserve(files.size());

  std::map<std::string, std::size_t> group_indices;
  std::map<std::string, std::size_t> group_seen_counts;
  std::size_t next_group_index = 1;

  for (std::size_t file_pos = 0; file_pos < files.size(); ++file_pos) {
    const fs::path& file_path = files[file_pos];
    std::error_code relative_error;
    const fs::path kRelativePath =
        fs::relative(file_path, input_root_path, relative_error);
    if (relative_error) {
      return {MakeError(FileCryptoError::kInvalidArgument,
                        "Failed to compute relative path for scanned file."),
              {}};
    }

    std::error_code size_error;
    const auto kFileSize = fs::file_size(file_path, size_error);
    if (size_error) {
      return {MakeError(FileCryptoError::kInputReadFailed,
                        "Failed to read input file size during plan build."),
              {}};
    }

    DirectoryTaskPlanEntry plan_entry{};
    plan_entry.input_path = file_path;
    plan_entry.output_path = output_root_path / kRelativePath;
    plan_entry.output_path.replace_extension(output_extension_lower);
    plan_entry.group_label =
        ResolveTopLevelGroupLabel(input_root_path, file_path);
    plan_entry.input_size_bytes = kFileSize;

    if (!group_indices.contains(plan_entry.group_label)) {
      group_indices[plan_entry.group_label] = next_group_index++;
    }
    plan_entry.group_index = group_indices[plan_entry.group_label];
    plan_entry.group_file_count = group_file_counts[plan_entry.group_label];
    ++group_seen_counts[plan_entry.group_label];
    plan_entry.group_file_index = group_seen_counts[plan_entry.group_label];
    plan_entry.file_index = file_pos + 1;
    plan_entry.total_files = files.size();

    plan.entries.push_back(std::move(plan_entry));
  }

  return {{}, std::move(plan)};
}

}  // namespace tracer_core::infrastructure::crypto::internal
