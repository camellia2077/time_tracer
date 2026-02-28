// infrastructure/crypto/internal/file_crypto_directory_plan.hpp
#ifndef INFRASTRUCTURE_CRYPTO_INTERNAL_FILE_CRYPTO_DIRECTORY_PLAN_HPP_
#define INFRASTRUCTURE_CRYPTO_INTERNAL_FILE_CRYPTO_DIRECTORY_PLAN_HPP_

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "infrastructure/crypto/file_crypto_service.hpp"

namespace tracer_core::infrastructure::crypto::internal {

namespace fs = std::filesystem;

struct DirectoryTaskPlanEntry {
  fs::path input_path;
  fs::path output_path;
  std::string group_label;
  std::uint64_t input_size_bytes = 0;
  std::size_t group_index = 0;
  std::size_t group_file_index = 0;
  std::size_t group_file_count = 0;
  std::size_t file_index = 0;
  std::size_t total_files = 0;
};

struct DirectoryTaskPlan {
  std::vector<DirectoryTaskPlanEntry> entries;
  std::size_t group_count = 0;
  std::uint64_t total_input_bytes = 0;
};

auto BuildSingleFilePlanEntry(const fs::path& input_path,
                              const fs::path& output_path)
    -> std::pair<FileCryptoResult, DirectoryTaskPlanEntry>;

auto BuildDirectoryTaskPlan(const fs::path& input_root_path,
                            const fs::path& output_root_path,
                            std::string_view input_extension_lower,
                            std::string_view output_extension_lower)
    -> std::pair<FileCryptoResult, DirectoryTaskPlan>;

}  // namespace tracer_core::infrastructure::crypto::internal

#endif  // INFRASTRUCTURE_CRYPTO_INTERNAL_FILE_CRYPTO_DIRECTORY_PLAN_HPP_
