// infrastructure/io/file_manager.cpp
#include "infrastructure/io/file_manager.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

#include "common/ansi_colors.hpp"

auto FileManager::setup_directories(const std::string& master_dir,
                                    int start_year, int end_year) -> bool {
  try {
    if (!std::filesystem::exists(master_dir)) {
      std::filesystem::create_directory(master_dir);
      std::cout << "Created master directory: '" << master_dir << "'\n";
    }
    for (int year = start_year; year <= end_year; ++year) {
      std::filesystem::path year_dir_path =
          std::filesystem::path(master_dir) / std::to_string(year);
      if (!std::filesystem::exists(year_dir_path)) {
        std::filesystem::create_directory(year_dir_path);
      }
    }
  } catch (const std::filesystem::filesystem_error& e) {
    std::cerr << RED_COLOR << "Error creating directories. Detail: " << e.what()
              << RESET_COLOR << '\n';
    return false;
  }
  return true;
}

auto FileManager::write_log_file(const std::filesystem::path& file_path,
                                 const std::string& content) -> bool {
  std::ofstream out_file(file_path);
  if (!out_file.is_open()) {
    std::cerr << RED_COLOR << "Error: Could not open file '"
              << file_path.string() << "' for writing." << RESET_COLOR << '\n';
    return false;
  }
  out_file << content;
  return true;
}

auto FileManager::read_file(const std::filesystem::path& file_path)
    -> std::optional<std::string> {
  std::ifstream in_file(file_path);
  if (!in_file.is_open()) {
    std::cerr << RED_COLOR << "Error: Could not open file '"
              << file_path.string() << "' for reading." << RESET_COLOR << '\n';
    return std::nullopt;
  }
  std::stringstream buffer;
  buffer << in_file.rdbuf();
  return buffer.str();
}
