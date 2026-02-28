// api/cli/impl/presentation/progress/crypto_progress_renderer.hpp
#pragma once

#include <cstddef>
#include <string>
#include <string_view>

#include "infrastructure/crypto/file_crypto_service.hpp"

namespace tracer_core::cli::impl::presentation::progress {

class CryptoProgressRenderer {
public:
  enum class OutputMode {
    kSingleLine = 0,
    kPlainText,
  };

  CryptoProgressRenderer() = default;

  [[nodiscard]] auto BuildCallback()
      -> tracer_core::infrastructure::crypto::FileCryptoProgressCallback;

private:
  [[nodiscard]] static auto DetectOutputMode() -> OutputMode;
  [[nodiscard]] static auto DetectCursorUpSupport() -> bool;

  [[nodiscard]] static auto ToOperationLabel(
      tracer_core::infrastructure::crypto::FileCryptoOperation operation)
      -> std::string_view;
  [[nodiscard]] auto BuildOverallLine(
      const tracer_core::infrastructure::crypto::FileCryptoProgressSnapshot
          &snapshot) const -> std::string;
  [[nodiscard]] auto BuildFolderLine(
      const tracer_core::infrastructure::crypto::FileCryptoProgressSnapshot
          &snapshot) const -> std::string;
  auto RenderTwoLine(std::string_view overall_line,
                     std::string_view folder_line, bool finish_line) -> void;
  auto RenderPlainText(std::string_view overall_line,
                       std::string_view folder_line, bool finish_line) -> void;

  [[nodiscard]] auto HandleProgress(
      const tracer_core::infrastructure::crypto::FileCryptoProgressSnapshot
          &snapshot) -> tracer_core::infrastructure::crypto::FileCryptoControl;

  OutputMode output_mode_ = DetectOutputMode();
  bool cursor_up_supported_ = DetectCursorUpSupport();
  bool has_active_render_ = false;
  std::size_t last_overall_width_ = 0;
  std::size_t last_folder_width_ = 0;
  std::string last_overall_text_;
  std::string last_folder_text_;
};

} // namespace tracer_core::cli::impl::presentation::progress
