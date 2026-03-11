[[nodiscard]] auto BuildReportChartSeries(
    std::string_view start_date, std::string_view end_date,
    const std::vector<DayDurationRow>& sparse_rows)
    -> ReportChartSeriesResult;
