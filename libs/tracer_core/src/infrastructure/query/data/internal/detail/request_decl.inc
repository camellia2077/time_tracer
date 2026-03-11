auto TrimCopy(std::string_view value) -> std::string;

auto NormalizeProjectRootFilter(const std::optional<std::string>& project)
    -> std::optional<std::string>;

auto ParseIsoDate(std::string_view value)
    -> std::optional<std::chrono::year_month_day>;

auto FormatIsoDate(const std::chrono::year_month_day& ymd) -> std::string;

[[nodiscard]] auto ResolveCurrentSystemLocalDate()
    -> std::chrono::year_month_day;

auto ResolvePositiveLookbackDays(const std::optional<int>& candidate,
                                 int fallback, std::string_view field_name)
    -> int;

auto NormalizeBoundaryDate(std::string_view input, bool is_end) -> std::string;

auto EnsureDbConnectionOrThrow(DBManager& db_manager,
                               const std::filesystem::path& db_path)
    -> sqlite3*;

auto ToCliDataQueryAction(tracer_core::core::dto::DataQueryAction action)
    -> tracer::core::infrastructure::query::data::DataQueryAction;

auto BuildCliFilters(const tracer_core::core::dto::DataQueryRequest& request)
    -> tracer::core::infrastructure::query::data::QueryFilters;
