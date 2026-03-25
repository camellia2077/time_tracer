auto HandleYearsQuery(sqlite3* db_conn,
                      tracer_core::core::dto::DataQueryOutputMode output_mode)
    -> tracer_core::core::dto::TextOutput;

auto HandleMonthsQuery(sqlite3* db_conn, const QueryFilters& base_filters,
                       tracer_core::core::dto::DataQueryOutputMode output_mode)
    -> tracer_core::core::dto::TextOutput;

auto HandleDaysQuery(sqlite3* db_conn, const QueryFilters& base_filters,
                     tracer_core::core::dto::DataQueryOutputMode output_mode)
    -> tracer_core::core::dto::TextOutput;

auto HandleDaysDurationQuery(
    sqlite3* db_conn, const QueryFilters& base_filters,
    tracer_core::core::dto::DataQueryOutputMode output_mode)
    -> tracer_core::core::dto::TextOutput;

auto HandleSearchQuery(sqlite3* db_conn, const QueryFilters& base_filters,
                       tracer_core::core::dto::DataQueryOutputMode output_mode)
    -> tracer_core::core::dto::TextOutput;

auto HandleActivitySuggestQuery(
    sqlite3* db_conn, const tracer_core::core::dto::DataQueryRequest& request,
    tracer_core::core::dto::DataQueryOutputMode output_mode)
    -> tracer_core::core::dto::TextOutput;
