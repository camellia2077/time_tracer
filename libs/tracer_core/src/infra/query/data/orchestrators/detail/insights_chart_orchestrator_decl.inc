auto HandleInsightsChartQuery(
    sqlite3* db_conn, const tracer_core::core::dto::DataQueryRequest& request,
    tracer_core::core::dto::DataQueryOutputMode output_mode)
    -> tracer_core::core::dto::TextOutput;

auto HandleInsightsCompositionQuery(
    sqlite3* db_conn, const tracer_core::core::dto::DataQueryRequest& request,
    tracer_core::core::dto::DataQueryOutputMode output_mode)
    -> tracer_core::core::dto::TextOutput;
