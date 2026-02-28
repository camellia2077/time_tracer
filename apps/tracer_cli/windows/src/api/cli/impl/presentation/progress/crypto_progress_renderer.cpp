// api/cli/impl/presentation/progress/crypto_progress_renderer.cpp
#include "api/cli/impl/presentation/progress/crypto_progress_renderer.hpp"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>

#if defined(_WIN32) || defined(_WIN64)
#include <io.h>
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace tracer_core::cli::impl::presentation::progress {
namespace file_crypto = tracer_core::infrastructure::crypto;
namespace {

[[nodiscard]] auto ClampPercent(std::uint64_t done_bytes,
                                std::uint64_t total_bytes) -> std::uint64_t {
  if (total_bytes == 0) {
    return 0;
  }
  if (done_bytes > total_bytes) {
    done_bytes = total_bytes;
  }
  return (done_bytes * 100U) / total_bytes;
}

[[nodiscard]] auto IsTruthyEnv(std::string_view value) -> bool {
  return value == "1" || value == "true" || value == "TRUE" || value == "on" ||
         value == "ON";
}

[[nodiscard]] auto FormatDecimalOneDigit(long double value) -> std::string {
  const std::uint64_t scaled10 =
      static_cast<std::uint64_t>((value * 10.0L) + 0.5L);
  const std::uint64_t integer_part = scaled10 / 10U;
  const std::uint64_t fraction_part = scaled10 % 10U;
  if (fraction_part == 0) {
    return std::to_string(integer_part);
  }
  return std::to_string(integer_part) + "." + std::to_string(fraction_part);
}

[[nodiscard]] auto FormatBytes(std::uint64_t bytes) -> std::string {
  constexpr std::string_view kUnits[] = {"B", "KB", "MB", "GB", "TB"};
  long double value = static_cast<long double>(bytes);
  std::size_t unit_index = 0;
  while (value >= 1024.0L && unit_index + 1 < std::size(kUnits)) {
    value /= 1024.0L;
    ++unit_index;
  }
  if (unit_index == 0) {
    return std::to_string(bytes) + "B";
  }
  return FormatDecimalOneDigit(value) + std::string(kUnits[unit_index]);
}

[[nodiscard]] auto Pad2(std::uint64_t value) -> std::string {
  if (value < 10U) {
    return "0" + std::to_string(value);
  }
  return std::to_string(value);
}

[[nodiscard]] auto FormatEta(std::uint64_t eta_seconds,
                             std::uint64_t remaining_bytes,
                             file_crypto::FileCryptoPhase phase)
    -> std::string {
  if (phase == file_crypto::FileCryptoPhase::kCompleted ||
      remaining_bytes == 0) {
    return "00:00";
  }
  if (eta_seconds == 0) {
    return "--:--";
  }
  if (eta_seconds >= 3600U) {
    const std::uint64_t hours = eta_seconds / 3600U;
    const std::uint64_t minutes = (eta_seconds % 3600U) / 60U;
    const std::uint64_t seconds = eta_seconds % 60U;
    return std::to_string(hours) + ":" + Pad2(minutes) + ":" + Pad2(seconds);
  }
  const std::uint64_t minutes = eta_seconds / 60U;
  const std::uint64_t seconds = eta_seconds % 60U;
  return Pad2(minutes) + ":" + Pad2(seconds);
}

[[nodiscard]] auto ResolveStatusText(file_crypto::FileCryptoPhase phase)
    -> std::string_view {
  switch (phase) {
  case file_crypto::FileCryptoPhase::kCompleted:
    return "success";
  case file_crypto::FileCryptoPhase::kCancelled:
    return "cancelled";
  case file_crypto::FileCryptoPhase::kFailed:
    return "failed";
  default:
    return "running";
  }
}

[[nodiscard]] auto ResolveCurrentFilePercent(
    const file_crypto::FileCryptoProgressSnapshot &snapshot)
    -> std::optional<std::uint64_t> {
  if (snapshot.current_file_total_bytes > 0) {
    return ClampPercent(snapshot.current_file_done_bytes,
                        snapshot.current_file_total_bytes);
  }
  if (snapshot.file_count_in_group > 0) {
    return static_cast<std::uint64_t>((snapshot.file_index_in_group * 100U) /
                                      snapshot.file_count_in_group);
  }
  return std::nullopt;
}

[[nodiscard]] auto ResolveCurrentFileFraction(
    const file_crypto::FileCryptoProgressSnapshot &snapshot) -> long double {
  const auto current_file_percent = ResolveCurrentFilePercent(snapshot);
  if (current_file_percent.has_value()) {
    return static_cast<long double>(*current_file_percent) / 100.0L;
  }
  if (snapshot.phase == file_crypto::FileCryptoPhase::kCompleted) {
    return 1.0L;
  }
  return 0.0L;
}

[[nodiscard]] auto
ResolveGroupPercent(const file_crypto::FileCryptoProgressSnapshot &snapshot)
    -> std::optional<std::uint64_t> {
  if (snapshot.file_count_in_group == 0) {
    return std::nullopt;
  }

  const std::size_t completed_files =
      snapshot.file_index_in_group > 0 ? snapshot.file_index_in_group - 1 : 0;
  const long double denominator =
      static_cast<long double>(snapshot.file_count_in_group);
  long double ratio = (static_cast<long double>(completed_files) +
                       ResolveCurrentFileFraction(snapshot)) /
                      denominator;

  if (snapshot.phase == file_crypto::FileCryptoPhase::kCompleted) {
    ratio = 1.0L;
  }
  ratio = std::clamp(ratio, 0.0L, 1.0L);
  return static_cast<std::uint64_t>(ratio * 100.0L);
}

[[nodiscard]] auto ResolveOverallFolderPercent(
    const file_crypto::FileCryptoProgressSnapshot &snapshot)
    -> std::optional<std::uint64_t> {
  if (snapshot.group_count == 0) {
    return std::nullopt;
  }

  const std::size_t completed_groups =
      snapshot.group_index > 0 ? snapshot.group_index - 1 : 0;
  const long double denominator =
      static_cast<long double>(snapshot.group_count);
  long double ratio =
      (static_cast<long double>(completed_groups) +
       (static_cast<long double>(ResolveGroupPercent(snapshot).value_or(0U)) /
        100.0L)) /
      denominator;

  if (snapshot.phase == file_crypto::FileCryptoPhase::kCompleted) {
    ratio = 1.0L;
  }
  ratio = std::clamp(ratio, 0.0L, 1.0L);
  return static_cast<std::uint64_t>(ratio * 100.0L);
}

[[nodiscard]] auto ResolveCompletedFileCount(std::size_t current_index,
                                             std::size_t total_count,
                                             std::uint64_t current_done_bytes,
                                             std::uint64_t current_total_bytes,
                                             file_crypto::FileCryptoPhase phase)
    -> std::size_t {
  if (total_count == 0) {
    return 0;
  }
  if (phase == file_crypto::FileCryptoPhase::kCompleted) {
    return total_count;
  }
  if (current_index == 0) {
    return 0;
  }

  std::size_t completed = current_index - 1;
  if (current_total_bytes > 0 && current_done_bytes >= current_total_bytes) {
    completed = current_index;
  }
  if (completed > total_count) {
    completed = total_count;
  }
  return completed;
}

[[nodiscard]] auto BuildAsciiBar(std::uint64_t percent, std::size_t width)
    -> std::string {
  if (percent > 100) {
    percent = 100;
  }
  const std::size_t filled = static_cast<std::size_t>((percent * width) / 100U);

  std::string bar;
  bar.reserve(width + 2);
  bar.push_back('[');
  for (std::size_t i = 0; i < width; ++i) {
    if (i < filled) {
      bar.push_back('=');
      continue;
    }
    if (i == filled && percent < 100) {
      bar.push_back('>');
      continue;
    }
    bar.push_back(' ');
  }
  bar.push_back(']');
  return bar;
}

} // namespace

auto CryptoProgressRenderer::DetectOutputMode() -> OutputMode {
  if (const char *force_plain = std::getenv("TT_CLI_PROGRESS_FORCE_PLAIN");
      force_plain != nullptr && IsTruthyEnv(force_plain)) {
    return OutputMode::kPlainText;
  }
  if (const char *force_single =
          std::getenv("TT_CLI_PROGRESS_FORCE_SINGLE_LINE");
      force_single != nullptr && IsTruthyEnv(force_single)) {
    return OutputMode::kSingleLine;
  }

#if defined(_WIN32) || defined(_WIN64)
  const int stdout_fd = _fileno(stdout);
  if (stdout_fd < 0 || _isatty(stdout_fd) == 0) {
    return OutputMode::kPlainText;
  }
#else
  if (isatty(fileno(stdout)) == 0) {
    return OutputMode::kPlainText;
  }
  if (const char *term = std::getenv("TERM");
      term != nullptr && std::string_view(term) == "dumb") {
    return OutputMode::kPlainText;
  }
#endif
  return OutputMode::kSingleLine;
}

auto CryptoProgressRenderer::DetectCursorUpSupport() -> bool {
  if (const char *disable_cursor_up =
          std::getenv("TT_CLI_PROGRESS_NO_CURSOR_UP");
      disable_cursor_up != nullptr && IsTruthyEnv(disable_cursor_up)) {
    return false;
  }

#if defined(_WIN32) || defined(_WIN64)
  HANDLE stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
  if (stdout_handle == nullptr || stdout_handle == INVALID_HANDLE_VALUE) {
    return false;
  }
  DWORD mode = 0;
  if (GetConsoleMode(stdout_handle, &mode) == 0) {
    return false;
  }
  if ((mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING) == 0) {
    const DWORD vt_mode = mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (SetConsoleMode(stdout_handle, vt_mode) != 0) {
      mode = vt_mode;
    }
  }
  return (mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING) != 0;
#else
  if (const char *term = std::getenv("TERM");
      term != nullptr && std::string_view(term) == "dumb") {
    return false;
  }
  return true;
#endif
}

auto CryptoProgressRenderer::BuildCallback()
    -> file_crypto::FileCryptoProgressCallback {
  return [this](const file_crypto::FileCryptoProgressSnapshot &snapshot)
             -> file_crypto::FileCryptoControl {
    return HandleProgress(snapshot);
  };
}

auto CryptoProgressRenderer::ToOperationLabel(
    file_crypto::FileCryptoOperation operation) -> std::string_view {
  switch (operation) {
  case file_crypto::FileCryptoOperation::kEncrypt:
    return "encrypt";
  case file_crypto::FileCryptoOperation::kDecrypt:
    return "decrypt";
  }
  return "unknown";
}

auto CryptoProgressRenderer::BuildOverallLine(
    const file_crypto::FileCryptoProgressSnapshot &snapshot) const
    -> std::string {
  const std::uint64_t overall_percent =
      ResolveOverallFolderPercent(snapshot).value_or(
          snapshot.overall_total_bytes > 0
              ? ClampPercent(snapshot.overall_done_bytes,
                             snapshot.overall_total_bytes)
              : 0U);

  const std::size_t done_txt = ResolveCompletedFileCount(
      snapshot.current_file_index, snapshot.total_files,
      snapshot.current_file_done_bytes, snapshot.current_file_total_bytes,
      snapshot.phase);

  std::string line;
  line.reserve(180);
  line += "[";
  line += ToOperationLabel(snapshot.operation);
  line += "] all ";
  line += BuildAsciiBar(overall_percent, 24);
  line += " ";
  line += std::to_string(overall_percent);
  line += "%";
  line += " | txt ";
  line += std::to_string(done_txt);
  line += "/";
  line += std::to_string(snapshot.total_files);
  line += " | speed ";
  line += FormatBytes(snapshot.speed_bytes_per_sec);
  line += "/s";
  line += " | remain ";
  line += FormatBytes(snapshot.remaining_bytes);
  line += " | eta ";
  line +=
      FormatEta(snapshot.eta_seconds, snapshot.remaining_bytes, snapshot.phase);
  return line;
}

auto CryptoProgressRenderer::BuildFolderLine(
    const file_crypto::FileCryptoProgressSnapshot &snapshot) const
    -> std::string {
  const std::size_t done_txt_in_group = ResolveCompletedFileCount(
      snapshot.file_index_in_group, snapshot.file_count_in_group,
      snapshot.current_file_done_bytes, snapshot.current_file_total_bytes,
      snapshot.phase);

  if (snapshot.group_index == 0 || snapshot.file_count_in_group == 0) {
    std::string preparing_line;
    preparing_line.reserve(128);
    preparing_line += "          folder (preparing)";
    if (snapshot.group_count > 0) {
      preparing_line += " (0/";
      preparing_line += std::to_string(snapshot.group_count);
      preparing_line += ")";
    }
    preparing_line += " ";
    preparing_line += BuildAsciiBar(0, 14);
    preparing_line += " 0%";
    preparing_line += " | txt 0/";
    preparing_line += std::to_string(snapshot.file_count_in_group);
    return preparing_line;
  }

  const std::string folder_label = snapshot.current_group_label.empty()
                                       ? "(root)"
                                       : snapshot.current_group_label;
  const std::size_t group_index = snapshot.group_index;
  const std::size_t group_count = snapshot.group_count;
  const std::uint64_t folder_percent =
      ResolveGroupPercent(snapshot).value_or(0U);

  std::string line;
  line.reserve(120);
  line += "          ";
  line += "folder ";
  line += folder_label;

  if (group_count > 0 && group_index > 0) {
    line += " (";
    line += std::to_string(group_index);
    line += "/";
    line += std::to_string(group_count);
    line += ")";
  }

  line += " ";
  line += BuildAsciiBar(folder_percent, 14);
  line += " ";
  line += std::to_string(folder_percent);
  line += "%";
  line += " | txt ";
  line += std::to_string(done_txt_in_group);
  line += "/";
  line += std::to_string(snapshot.file_count_in_group);

  if (snapshot.phase == file_crypto::FileCryptoPhase::kCompleted ||
      snapshot.phase == file_crypto::FileCryptoPhase::kCancelled ||
      snapshot.phase == file_crypto::FileCryptoPhase::kFailed) {
    line += " | ";
    line += ResolveStatusText(snapshot.phase);
  }
  return line;
}

auto CryptoProgressRenderer::RenderTwoLine(std::string_view overall_line,
                                           std::string_view folder_line,
                                           bool finish_line) -> void {
  if (!has_active_render_) {
    std::cout << overall_line << '\n' << folder_line;
  } else if (cursor_up_supported_) {
    std::cout << '\r' << "\x1b[1A";
    std::cout << overall_line;
    if (overall_line.size() < last_overall_width_) {
      std::cout << std::string(last_overall_width_ - overall_line.size(), ' ');
    }
    std::cout << '\n';
    std::cout << folder_line;
    if (folder_line.size() < last_folder_width_) {
      std::cout << std::string(last_folder_width_ - folder_line.size(), ' ');
    }
  } else {
    if (overall_line != last_overall_text_) {
      std::cout << '\n' << overall_line << '\n';
      std::cout << folder_line;
    } else {
      std::cout << '\r' << folder_line;
    }
    if (folder_line.size() < last_folder_width_) {
      std::cout << std::string(last_folder_width_ - folder_line.size(), ' ');
    }
  }

  std::cout.flush();
  has_active_render_ = true;
  last_overall_width_ = std::max(last_overall_width_, overall_line.size());
  last_folder_width_ = std::max(last_folder_width_, folder_line.size());
  last_overall_text_.assign(overall_line);
  last_folder_text_.assign(folder_line);

  if (finish_line) {
    std::cout << '\n';
    std::cout.flush();
    has_active_render_ = false;
    last_overall_width_ = 0;
    last_folder_width_ = 0;
    last_overall_text_.clear();
    last_folder_text_.clear();
  }
}

auto CryptoProgressRenderer::RenderPlainText(std::string_view overall_line,
                                             std::string_view folder_line,
                                             bool finish_line) -> void {
  if (overall_line != last_overall_text_) {
    std::cout << overall_line << '\n';
    last_overall_text_.assign(overall_line);
  }

  if (finish_line || folder_line != last_folder_text_) {
    std::cout << folder_line << '\n';
    last_folder_text_.assign(folder_line);
  }
  std::cout.flush();

  if (finish_line) {
    last_overall_text_.clear();
    last_folder_text_.clear();
  }
}

auto CryptoProgressRenderer::HandleProgress(
    const file_crypto::FileCryptoProgressSnapshot &snapshot)
    -> file_crypto::FileCryptoControl {
  const bool is_terminal_phase =
      snapshot.phase == file_crypto::FileCryptoPhase::kCompleted ||
      snapshot.phase == file_crypto::FileCryptoPhase::kCancelled ||
      snapshot.phase == file_crypto::FileCryptoPhase::kFailed;

  const std::string overall_line = BuildOverallLine(snapshot);
  const std::string folder_line = BuildFolderLine(snapshot);

  if (!is_terminal_phase && overall_line == last_overall_text_ &&
      folder_line == last_folder_text_) {
    return file_crypto::FileCryptoControl::kContinue;
  }

  if (output_mode_ == OutputMode::kSingleLine) {
    RenderTwoLine(overall_line, folder_line, is_terminal_phase);
    return file_crypto::FileCryptoControl::kContinue;
  }

  RenderPlainText(overall_line, folder_line, is_terminal_phase);
  return file_crypto::FileCryptoControl::kContinue;
}

} // namespace tracer_core::cli::impl::presentation::progress
