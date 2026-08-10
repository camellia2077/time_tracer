auto ReadToml(const fs::path& path) -> toml::table;

template <typename T>
auto GetOptional(const toml::node_view<const toml::node>& node,
                 const T& default_value) -> T {
  return node.value_or(default_value);
}

template <typename T>
auto GetRequired(const toml::table& tbl, const std::string& key) -> T {
  if (!tbl.contains(key)) {
    throw std::runtime_error("Missing required config key: " + key);
  }
  auto val = tbl[key].value<T>();
  if (!val) {
    throw std::runtime_error("Invalid type for key: " + key);
  }
  return *val;
}

void ParseStatisticsItems(const toml::array* arr,
                          std::vector<InsightsStatisticsItem>& out_items);

void FillTexStyle(const toml::table& tbl, FontConfig& fonts,
                  LayoutConfig& layout);
void FillTypStyle(const toml::table& tbl, FontConfig& fonts,
                  LayoutConfig& layout);

void FillDailyLabels(const toml::table& tbl, DailyInsightsLabels& labels);
void FillRangeLabels(const toml::table& tbl, RangeInsightsLabels& labels);
void FillMonthlyLabels(const toml::table& tbl, MonthlyInsightsLabels& labels);
void FillPeriodLabels(const toml::table& tbl, PeriodInsightsLabels& labels);
void FillWeeklyLabels(const toml::table& tbl, WeeklyInsightsLabels& labels);
void FillYearlyLabels(const toml::table& tbl, YearlyInsightsLabels& labels);

void FillKeywordColors(const toml::table& tbl,
                       std::map<std::string, std::string>& colors);
