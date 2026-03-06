namespace {

auto IsHexColor(const std::string& value) -> bool {
  constexpr std::size_t kHexColorLength = 7;
  if (value.size() != kHexColorLength || value.front() != '#') {
    return false;
  }
  return std::all_of(value.begin() + 1, value.end(),
                     [](unsigned char code_point) -> bool {
                       return std::isxdigit(code_point) != 0;
                     });
}

auto ValidatePositiveHours(const toml::table& thresholds_table,
                           const fs::path& heatmap_path) -> std::size_t {
  const toml::node* positive_hours_node =
      thresholds_table.get("positive_hours");
  if (positive_hours_node == nullptr) {
    ThrowConfigFieldError(heatmap_path, "thresholds.positive_hours",
                          "is required and must be an array.");
  }

  const toml::array* positive_hours_array = positive_hours_node->as_array();
  if (positive_hours_array == nullptr || positive_hours_array->empty()) {
    ThrowConfigFieldError(heatmap_path, "thresholds.positive_hours",
                          "must be a non-empty array.");
  }

  std::vector<double> positive_hours;
  positive_hours.reserve(positive_hours_array->size());
  for (std::size_t index = 0; index < positive_hours_array->size(); ++index) {
    const std::string kFieldPath =
        "thresholds.positive_hours[" + std::to_string(index) + "]";
    const toml::node* item = positive_hours_array->get(index);
    if (item == nullptr) {
      ThrowConfigFieldError(heatmap_path, kFieldPath,
                            "must be a positive number.");
    }
    const auto kValue = item->value<double>();
    if (!kValue.has_value() || !std::isfinite(*kValue) || *kValue <= 0.0) {
      ThrowConfigFieldError(heatmap_path, kFieldPath,
                            "must be a positive number.");
    }
    if (!positive_hours.empty() && *kValue <= positive_hours.back()) {
      ThrowConfigFieldError(heatmap_path, kFieldPath,
                            "must be strictly increasing.");
    }
    positive_hours.push_back(*kValue);
  }

  return positive_hours.size() + 1U;
}

auto ValidatePalettes(const toml::table& palettes_table,
                      const fs::path& heatmap_path,
                      std::size_t expected_palette_size,
                      const std::string& light_palette,
                      const std::string& dark_palette) -> void {
  bool has_light_palette = false;
  bool has_dark_palette = false;
  for (const auto& [key, node] : palettes_table) {
    const std::string kPaletteName = std::string(key.str());
    if (kPaletteName == light_palette) {
      has_light_palette = true;
    }
    if (kPaletteName == dark_palette) {
      has_dark_palette = true;
    }

    const toml::array* colors = node.as_array();
    if (colors == nullptr) {
      ThrowConfigFieldError(heatmap_path, "palettes." + kPaletteName,
                            "must be an array of hex colors.");
    }
    if (colors->size() != expected_palette_size) {
      ThrowConfigFieldError(heatmap_path, "palettes." + kPaletteName,
                            "must contain exactly " +
                                std::to_string(expected_palette_size) +
                                " colors to match thresholds.positive_hours.");
    }
    for (std::size_t index = 0; index < colors->size(); ++index) {
      const std::string kFieldPath =
          "palettes." + kPaletteName + "[" + std::to_string(index) + "]";
      const toml::node* color_node = colors->get(index);
      const auto kColorValue = color_node == nullptr
                                   ? std::optional<std::string>{}
                                   : color_node->value<std::string>();
      if (!kColorValue.has_value() || !IsHexColor(*kColorValue)) {
        ThrowConfigFieldError(heatmap_path, kFieldPath,
                              "must be a hex color (#RRGGBB).");
      }
    }
  }

  if (!has_light_palette) {
    ThrowConfigFieldError(heatmap_path, "defaults.light_palette",
                          "must reference an existing palette key.");
  }
  if (!has_dark_palette) {
    ThrowConfigFieldError(heatmap_path, "defaults.dark_palette",
                          "must reference an existing palette key.");
  }
}

auto ValidateHeatmapConfigFile(const fs::path& heatmap_path) -> void {
  toml::table table;
  try {
    table = toml::parse_file(heatmap_path.string());
  } catch (const toml::parse_error& err) {
    ThrowConfigFieldError(
        heatmap_path, "",
        "failed to parse TOML: " + std::string(err.description()) + ".");
  }

  const auto kSchemaVersion = RequireTypedField<int64_t>(
      table, "schema_version", heatmap_path, "", "an integer");
  if (kSchemaVersion != 1) {
    ThrowConfigFieldError(heatmap_path, "schema_version",
                          "must be 1 for current runtime.");
  }

  const toml::table* thresholds_tbl =
      TryReadTableField(table, "thresholds", heatmap_path, "");
  if (thresholds_tbl == nullptr) {
    ThrowConfigFieldError(heatmap_path, "thresholds",
                          "is required and must be a table.");
  }
  const std::size_t kExpectedPaletteSize =
      ValidatePositiveHours(*thresholds_tbl, heatmap_path);

  const toml::table* defaults_tbl =
      TryReadTableField(table, "defaults", heatmap_path, "");
  if (defaults_tbl == nullptr) {
    ThrowConfigFieldError(heatmap_path, "defaults",
                          "is required and must be a table.");
  }
  const std::string kLightPalette = RequireNonEmptyStringField(
      *defaults_tbl, "light_palette", heatmap_path, "defaults");
  const std::string kDarkPalette = RequireNonEmptyStringField(
      *defaults_tbl, "dark_palette", heatmap_path, "defaults");

  const toml::table* palettes_tbl =
      TryReadTableField(table, "palettes", heatmap_path, "");
  if (palettes_tbl == nullptr || palettes_tbl->empty()) {
    ThrowConfigFieldError(heatmap_path, "palettes",
                          "is required and must contain at least one palette.");
  }

  ValidatePalettes(*palettes_tbl, heatmap_path, kExpectedPaletteSize,
                   kLightPalette, kDarkPalette);
}

}  // namespace
