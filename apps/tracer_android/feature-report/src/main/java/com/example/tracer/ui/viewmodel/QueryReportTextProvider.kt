package com.example.tracer

import android.content.Context
import com.example.tracer.feature.report.R

interface QueryReportTextProvider {
    fun statsSubjectLabel(): String
    fun treeSubjectLabel(): String

    fun nativeReportRunning(mode: String): String
    fun nativeReportResult(mode: String, ok: Boolean): String
    fun rangeStartDateInvalid(detail: String): String
    fun rangeEndDateInvalid(detail: String): String
    fun queryStatsRunning(period: String): String
    fun queryStatsResult(period: String, ok: Boolean): String
    fun queryTreeRunning(): String
    fun queryTreeResult(ok: Boolean): String
    fun queryChartRunning(): String
    fun queryChartResult(ok: Boolean): String
    fun chartPayloadInvalid(): String
    fun chartRangeBothRequired(): String
    fun chartRangeStartDateInvalid(): String
    fun chartRangeEndDateInvalid(): String
    fun chartRangeOrderInvalid(): String
    fun treeLevelMustBeAtLeastMinusOne(): String

    fun periodArgumentRequired(subjectLabel: String, periodLabel: String): String
    fun invalidRangeArgumentFormat(): String
    fun invalidRangeDateValue(): String
    fun subjectRangeInvalid(subjectLabel: String): String

    fun invalidDayFormat(): String
    fun invalidDateValue(value: String): String
    fun invalidMonthFormat(): String
    fun invalidMonthValue(value: String): String
    fun invalidYearFormat(): String
    fun invalidIsoYearValue(value: String): String
    fun invalidWeekFormat(): String
    fun invalidWeekYear(value: String): String
    fun invalidWeekNumber(value: String): String
    fun invalidWeekValue(value: String, year: Int, maxIsoWeek: Int): String
    fun invalidRangeOrder(): String
    fun recentDaysRequired(): String
    fun recentDaysMustBeNumeric(): String
    fun recentDaysMustBeGreaterThanZero(): String
    fun unknownViewModelClass(className: String): String

    fun periodLabel(period: DataTreePeriod): String
}

object DefaultQueryReportTextProvider : QueryReportTextProvider {
    override fun statsSubjectLabel(): String = "Stats"
    override fun treeSubjectLabel(): String = "Tree"

    override fun nativeReportRunning(mode: String): String =
        "nativeReport($mode, md) running..."

    override fun nativeReportResult(mode: String, ok: Boolean): String =
        "nativeReport($mode, md) -> OK=$ok"

    override fun rangeStartDateInvalid(detail: String): String =
        "Range start date invalid. $detail"

    override fun rangeEndDateInvalid(detail: String): String =
        "Range end date invalid. $detail"

    override fun queryStatsRunning(period: String): String =
        "query data days-stats running... period=$period"

    override fun queryStatsResult(period: String, ok: Boolean): String =
        "query data days-stats($period) -> OK=$ok"

    override fun queryTreeRunning(): String = "query data tree running..."
    override fun queryTreeResult(ok: Boolean): String = "query data tree -> OK=$ok"
    override fun queryChartRunning(): String = "query data report-chart running..."
    override fun queryChartResult(ok: Boolean): String = "query data report-chart -> OK=$ok"
    override fun chartPayloadInvalid(): String = "report chart payload is invalid."
    override fun chartRangeBothRequired(): String =
        "Start and end date are required for range filter."

    override fun chartRangeStartDateInvalid(): String =
        "Invalid start date. Use YYYYMMDD."

    override fun chartRangeEndDateInvalid(): String =
        "Invalid end date. Use YYYYMMDD."

    override fun chartRangeOrderInvalid(): String =
        "Invalid range. Start date must be less than or equal to end date."

    override fun treeLevelMustBeAtLeastMinusOne(): String = "Tree level must be >= -1."

    override fun periodArgumentRequired(subjectLabel: String, periodLabel: String): String =
        "$subjectLabel argument is required for period $periodLabel."

    override fun invalidRangeArgumentFormat(): String =
        "Invalid range argument. Use start|end (example: 2026-02-01|2026-02-15)."

    override fun invalidRangeDateValue(): String =
        "Invalid range date value. Use YYYY-MM-DD (or YYYYMMDD)."

    override fun subjectRangeInvalid(subjectLabel: String): String =
        "$subjectLabel range invalid. Start date must be <= end date."

    override fun invalidDayFormat(): String =
        "Invalid day format. Use digits YYYYMMDD (example: 20260214)."

    override fun invalidDateValue(value: String): String =
        "Invalid date value: $value."

    override fun invalidMonthFormat(): String =
        "Invalid month format. Use digits YYYYMM (example: 202602)."

    override fun invalidMonthValue(value: String): String =
        "Invalid month value: $value."

    override fun invalidYearFormat(): String =
        "Invalid year format. Use ISO year: YYYY (example: 2026)."

    override fun invalidIsoYearValue(value: String): String =
        "Invalid ISO year value: $value."

    override fun invalidWeekFormat(): String =
        "Invalid week format. Use digits YYYYWW (example: 202607)."

    override fun invalidWeekYear(value: String): String =
        "Invalid week year: $value."

    override fun invalidWeekNumber(value: String): String =
        "Invalid week number: $value."

    override fun invalidWeekValue(value: String, year: Int, maxIsoWeek: Int): String =
        "Invalid week value: $value. For year $year, valid weeks are 01 to $maxIsoWeek."

    override fun invalidRangeOrder(): String =
        "Invalid range. Start date must be <= end date."

    override fun recentDaysRequired(): String =
        "Recent days is required and must be a number."

    override fun recentDaysMustBeNumeric(): String =
        "Recent days must be numeric."

    override fun recentDaysMustBeGreaterThanZero(): String =
        "Recent days must be greater than 0."

    override fun unknownViewModelClass(className: String): String =
        "Unknown ViewModel class: $className"

    override fun periodLabel(period: DataTreePeriod): String =
        period.wireValue
}

class AndroidQueryReportTextProvider(
    private val context: Context
) : QueryReportTextProvider {
    override fun statsSubjectLabel(): String =
        context.getString(R.string.report_subject_stats)

    override fun treeSubjectLabel(): String =
        context.getString(R.string.report_subject_tree)

    override fun nativeReportRunning(mode: String): String =
        context.getString(R.string.report_status_native_report_running, mode)

    override fun nativeReportResult(mode: String, ok: Boolean): String =
        context.getString(R.string.report_status_native_report_result, mode, ok.toString())

    override fun rangeStartDateInvalid(detail: String): String =
        context.getString(R.string.report_status_range_start_date_invalid, detail)

    override fun rangeEndDateInvalid(detail: String): String =
        context.getString(R.string.report_status_range_end_date_invalid, detail)

    override fun queryStatsRunning(period: String): String =
        context.getString(R.string.report_status_query_stats_running, period)

    override fun queryStatsResult(period: String, ok: Boolean): String =
        context.getString(R.string.report_status_query_stats_result, period, ok.toString())

    override fun queryTreeRunning(): String =
        context.getString(R.string.report_status_query_tree_running)

    override fun queryTreeResult(ok: Boolean): String =
        context.getString(R.string.report_status_query_tree_result, ok.toString())

    override fun queryChartRunning(): String =
        context.getString(R.string.report_status_query_chart_running)

    override fun queryChartResult(ok: Boolean): String =
        context.getString(R.string.report_status_query_chart_result, ok.toString())

    override fun chartPayloadInvalid(): String =
        context.getString(R.string.report_error_chart_payload_invalid)

    override fun chartRangeBothRequired(): String =
        context.getString(R.string.report_chart_range_error_both_required)

    override fun chartRangeStartDateInvalid(): String =
        context.getString(R.string.report_chart_range_error_invalid_start)

    override fun chartRangeEndDateInvalid(): String =
        context.getString(R.string.report_chart_range_error_invalid_end)

    override fun chartRangeOrderInvalid(): String =
        context.getString(R.string.report_chart_range_error_order)

    override fun treeLevelMustBeAtLeastMinusOne(): String =
        context.getString(R.string.report_error_tree_level_min)

    override fun periodArgumentRequired(subjectLabel: String, periodLabel: String): String =
        context.getString(R.string.report_error_period_argument_required, subjectLabel, periodLabel)

    override fun invalidRangeArgumentFormat(): String =
        context.getString(R.string.report_error_invalid_range_argument)

    override fun invalidRangeDateValue(): String =
        context.getString(R.string.report_error_invalid_range_date_value)

    override fun subjectRangeInvalid(subjectLabel: String): String =
        context.getString(R.string.report_error_subject_range_invalid, subjectLabel)

    override fun invalidDayFormat(): String =
        context.getString(R.string.report_error_invalid_day_format)

    override fun invalidDateValue(value: String): String =
        context.getString(R.string.report_error_invalid_date_value, value)

    override fun invalidMonthFormat(): String =
        context.getString(R.string.report_error_invalid_month_format)

    override fun invalidMonthValue(value: String): String =
        context.getString(R.string.report_error_invalid_month_value, value)

    override fun invalidYearFormat(): String =
        context.getString(R.string.report_error_invalid_year_format)

    override fun invalidIsoYearValue(value: String): String =
        context.getString(R.string.report_error_invalid_iso_year_value, value)

    override fun invalidWeekFormat(): String =
        context.getString(R.string.report_error_invalid_week_format)

    override fun invalidWeekYear(value: String): String =
        context.getString(R.string.report_error_invalid_week_year, value)

    override fun invalidWeekNumber(value: String): String =
        context.getString(R.string.report_error_invalid_week_number, value)

    override fun invalidWeekValue(value: String, year: Int, maxIsoWeek: Int): String =
        context.getString(R.string.report_error_invalid_week_value, value, year, maxIsoWeek)

    override fun invalidRangeOrder(): String =
        context.getString(R.string.report_error_invalid_range_order)

    override fun recentDaysRequired(): String =
        context.getString(R.string.report_error_recent_days_required)

    override fun recentDaysMustBeNumeric(): String =
        context.getString(R.string.report_error_recent_days_numeric)

    override fun recentDaysMustBeGreaterThanZero(): String =
        context.getString(R.string.report_error_recent_days_positive)

    override fun unknownViewModelClass(className: String): String =
        context.getString(R.string.report_error_unknown_viewmodel_class, className)

    override fun periodLabel(period: DataTreePeriod): String =
        context.getString(period.labelRes())
}
