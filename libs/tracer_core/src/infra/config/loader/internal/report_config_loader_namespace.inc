namespace {
constexpr int kDefaultStatisticFontSize = 10;
constexpr int kDefaultStatisticTitleFontSize = 12;
constexpr size_t kHexColorLength = 7;

[[noreturn]] auto ThrowInvalidConfig(const fs::path& path,
                                     std::string_view detail) -> void {
  throw std::runtime_error("Invalid report config [" + path.string() +
                           "]: " + std::string(detail));
}

auto RequireNode(const toml::table& tbl, const fs::path& path,
                 std::string_view key) -> toml::node_view<const toml::node> {
  const toml::node_view<const toml::node> kNode = tbl[key];
  if (!kNode) {
    ThrowInvalidConfig(path, "missing key '" + std::string(key) + "'.");
  }
  return kNode;
}

auto RequireString(const toml::table& tbl, const fs::path& path,
                   std::string_view key) -> void {
  const toml::node_view<const toml::node> kNode = RequireNode(tbl, path, key);
  if (!kNode.is_string()) {
    ThrowInvalidConfig(path,
                       "key '" + std::string(key) + "' must be a string.");
  }
  const auto kValue = kNode.value<std::string>();
  if (!kValue || kValue->empty()) {
    ThrowInvalidConfig(path,
                       "key '" + std::string(key) + "' must not be empty.");
  }
}

auto ValidateOptionalStringIfPresent(const toml::table& tbl,
                                     const fs::path& path, std::string_view key)
    -> void {
  const toml::node_view<const toml::node> kNode = tbl[key];
  if (!kNode) {
    return;
  }
  if (!kNode.is_string()) {
    ThrowInvalidConfig(
        path, "key '" + std::string(key) + "' must be a string when present.");
  }
  const auto kValue = kNode.value<std::string>();
  if (!kValue || kValue->empty()) {
    ThrowInvalidConfig(
        path, "key '" + std::string(key) + "' must not be empty when present.");
  }
}

auto RequirePositiveInteger(const toml::table& tbl, const fs::path& path,
                            std::string_view key) -> void {
  const toml::node_view<const toml::node> kNode = RequireNode(tbl, path, key);
  if (!kNode.is_integer()) {
    ThrowInvalidConfig(path,
                       "key '" + std::string(key) + "' must be an integer.");
  }
  const auto kValue = kNode.value<int>();
  if (!kValue || *kValue <= 0) {
    ThrowInvalidConfig(path, "key '" + std::string(key) + "' must be > 0.");
  }
}

auto ValidateOptionalPositiveIntegerIfPresent(const toml::table& tbl,
                                              const fs::path& path,
                                              std::string_view key) -> void {
  const toml::node_view<const toml::node> kNode = tbl[key];
  if (!kNode) {
    return;
  }
  if (!kNode.is_integer()) {
    ThrowInvalidConfig(path, "key '" + std::string(key) +
                                 "' must be an integer when present.");
  }
  const auto kValue = kNode.value<int>();
  if (!kValue || *kValue <= 0) {
    ThrowInvalidConfig(
        path, "key '" + std::string(key) + "' must be > 0 when present.");
  }
}

auto ValidateOptionalNumberIfPresent(const toml::table& tbl,
                                     const fs::path& path, std::string_view key)
    -> void {
  const toml::node_view<const toml::node> kNode = tbl[key];
  if (!kNode) {
    return;
  }
  if (!kNode.is_number()) {
    ThrowInvalidConfig(
        path, "key '" + std::string(key) + "' must be numeric when present.");
  }
}

auto IsValidHexColor(std::string_view color) -> bool {
  if (color.size() != kHexColorLength || color[0] != '#') {
    return false;
  }
  for (size_t index = 1; index < color.size(); ++index) {
    if (std::isxdigit(static_cast<unsigned char>(color[index])) == 0) {
      return false;
    }
  }
  return true;
}

auto ValidateStatisticsItemsArray(const toml::array& items,
                                  const fs::path& path,
                                  std::string_view context) -> void {
  if (items.empty()) {
    ThrowInvalidConfig(path, std::string(context) + " must not be empty.");
  }
  for (size_t index = 0; index < items.size(); ++index) {
    const toml::node& item_node = items[index];
    const std::string kItemContext =
        std::string(context) + "[" + std::to_string(index) + "]";
    if (!item_node.is_table()) {
      ThrowInvalidConfig(path, kItemContext + " must be a table.");
    }
    const toml::table& item_tbl = *item_node.as_table();

    const toml::node_view<const toml::node> kLabelNode = item_tbl["label"];
    if (!kLabelNode || !kLabelNode.is_string()) {
      ThrowInvalidConfig(path, kItemContext + ".label must be a string.");
    }
    const auto kLabelValue = kLabelNode.value<std::string>();
    if (!kLabelValue || kLabelValue->empty()) {
      ThrowInvalidConfig(path, kItemContext + ".label must not be empty.");
    }

    const toml::node_view<const toml::node> kShowNode = item_tbl["show"];
    if (kShowNode && !kShowNode.is_boolean()) {
      ThrowInvalidConfig(path, kItemContext + ".show must be a boolean.");
    }

    const toml::node_view<const toml::node> kColumnNode = item_tbl["db_column"];
    if (kColumnNode && !kColumnNode.is_string()) {
      ThrowInvalidConfig(path, kItemContext + ".db_column must be a string.");
    }

    const toml::node_view<const toml::node> kSubItemsNode =
        item_tbl["sub_items"];
    if (!kSubItemsNode) {
      continue;
    }
    if (!kSubItemsNode.is_array()) {
      ThrowInvalidConfig(path, kItemContext + ".sub_items must be an array.");
    }
    ValidateStatisticsItemsArray(*kSubItemsNode.as_array(), path,
                                 kItemContext + ".sub_items");
  }
}

auto ValidateRequiredPeriodLabels(const toml::table& tbl, const fs::path& path)
    -> void {
  RequireString(tbl, path, "title_template");
  RequireString(tbl, path, "actual_days_label");
  RequireString(tbl, path, "status_days_label");
  RequireString(tbl, path, "wake_anchor_days_label");
  RequireString(tbl, path, "exercise_days_label");
  RequireString(tbl, path, "cardio_days_label");
  RequireString(tbl, path, "anaerobic_days_label");
  RequireString(tbl, path, "total_time_label");
  RequireString(tbl, path, "no_records_message");
  RequireString(tbl, path, "invalid_range_message");
  RequireString(tbl, path, "project_breakdown_label");
}

auto ValidateDailyLabels(const toml::table& tbl, const fs::path& path,
                         std::string_view title_key) -> void {
  RequireString(tbl, path, title_key);
  RequireString(tbl, path, "date_label");
  RequireString(tbl, path, "total_time_label");
  RequireString(tbl, path, "status_label");
  RequireString(tbl, path, "wake_anchor_label");
  RequireString(tbl, path, "exercise_label");
  RequireString(tbl, path, "getup_time_label");
  RequireString(tbl, path, "remark_label");
  RequireString(tbl, path, "statistics_label");
  RequireString(tbl, path, "all_activities_label");
  RequireString(tbl, path, "activity_remark_label");
  RequireString(tbl, path, "activity_connector");
  RequireString(tbl, path, "no_records_message");
}

auto ValidateDailyStatistics(const toml::table& tbl, const fs::path& path)
    -> void {
  const toml::node_view<const toml::node> kStatsNode =
      RequireNode(tbl, path, "statistics_items");
  if (!kStatsNode.is_array()) {
    ThrowInvalidConfig(path, "key 'statistics_items' must be an array.");
  }
  ValidateStatisticsItemsArray(*kStatsNode.as_array(), path,
                               "statistics_items");
}

auto ValidateKeywordColors(const toml::table& tbl, const fs::path& path)
    -> void {
  const toml::node_view<const toml::node> kColorNode =
      RequireNode(tbl, path, "keyword_colors");
  if (!kColorNode.is_table()) {
    ThrowInvalidConfig(path, "key 'keyword_colors' must be a table.");
  }

  const toml::table& colors = *kColorNode.as_table();
  if (colors.empty()) {
    ThrowInvalidConfig(path, "key 'keyword_colors' must not be empty.");
  }

  for (const auto& [color_key, color_value_node] : colors) {
    if (!color_value_node.is_string()) {
      ThrowInvalidConfig(path, "keyword_colors." +
                                   std::string(color_key.str()) +
                                   " must be a string.");
    }
    const auto kColorValue = color_value_node.value<std::string>();
    if (!kColorValue || !IsValidHexColor(*kColorValue)) {
      ThrowInvalidConfig(path, "keyword_colors." +
                                   std::string(color_key.str()) +
                                   " must be a hex color like #RRGGBB.");
    }
  }
}

auto ValidateTexStyle(const toml::table& tbl, const fs::path& path) -> void {
  RequireString(tbl, path, "main_font");
  ValidateOptionalStringIfPresent(tbl, path, "cjk_main_font");
  RequirePositiveInteger(tbl, path, "base_font_size");
  RequirePositiveInteger(tbl, path, "report_title_font_size");
  RequirePositiveInteger(tbl, path, "category_title_font_size");
  ValidateOptionalNumberIfPresent(tbl, path, "margin_in");
  ValidateOptionalNumberIfPresent(tbl, path, "list_top_sep_pt");
  ValidateOptionalNumberIfPresent(tbl, path, "list_item_sep_ex");
}

auto ValidateTypStyle(const toml::table& tbl, const fs::path& path) -> void {
  RequireString(tbl, path, "base_font");
  ValidateOptionalStringIfPresent(tbl, path, "title_font");
  ValidateOptionalStringIfPresent(tbl, path, "category_title_font");
  RequirePositiveInteger(tbl, path, "base_font_size");
  RequirePositiveInteger(tbl, path, "report_title_font_size");
  RequirePositiveInteger(tbl, path, "category_title_font_size");
  ValidateOptionalNumberIfPresent(tbl, path, "line_spacing_em");
  ValidateOptionalNumberIfPresent(tbl, path, "margin_top_cm");
  ValidateOptionalNumberIfPresent(tbl, path, "margin_bottom_cm");
  ValidateOptionalNumberIfPresent(tbl, path, "margin_left_cm");
  ValidateOptionalNumberIfPresent(tbl, path, "margin_right_cm");
}

}  // namespace
