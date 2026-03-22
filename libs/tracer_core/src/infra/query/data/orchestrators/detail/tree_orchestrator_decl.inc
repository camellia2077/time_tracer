auto HandleTreeQuery(sqlite3* db_conn,
                     const tracer_core::core::dto::DataQueryRequest& request,
                     const QueryFilters& base_filters,
                     tracer_core::core::dto::DataQueryOutputMode output_mode)
    -> tracer_core::core::dto::TextOutput;
