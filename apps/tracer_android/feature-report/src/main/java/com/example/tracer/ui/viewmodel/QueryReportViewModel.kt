package com.example.tracer

import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.lifecycle.ViewModel
import androidx.lifecycle.ViewModelProvider
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.launch
import java.text.SimpleDateFormat
import java.util.Calendar
import java.util.Date
import java.util.Locale

sealed class QueryResult {
    data class Report(val text: String) : QueryResult()
    data class Stats(val text: String, val period: DataTreePeriod) : QueryResult()
    data class Tree(val text: String, val period: DataTreePeriod) : QueryResult()
}

data class QueryReportUiState(
    val reportDate: String = currentDateDigits(),
    val reportMonth: String = currentMonthDigits(),
    val reportYear: String = currentIsoYear(),
    val reportWeek: String = currentWeekDigits(),
    val reportRangeStartDate: String = currentMonthStartDateDigits(),
    val reportRangeEndDate: String = currentDateDigits(),
    val reportRecentDays: String = "7",
    val activeResult: QueryResult? = null,
    val statsPeriod: DataTreePeriod = DataTreePeriod.RECENT,
    val treePeriod: DataTreePeriod = DataTreePeriod.RECENT,
    val analysisLoading: Boolean = false,
    val analysisError: String = "",
    val statusText: String = ""
)

private fun currentDateDigits(): String =
    SimpleDateFormat("yyyyMMdd", Locale.US).format(Date())

private fun currentMonthDigits(): String =
    SimpleDateFormat("yyyyMM", Locale.US).format(Date())

private fun currentIsoYear(): String =
    Calendar.getInstance().get(Calendar.YEAR).toString()

private fun currentWeekDigits(): String {
    val cal = Calendar.getInstance(Locale.US).apply {
        time = Date()
        firstDayOfWeek = Calendar.MONDAY
        minimalDaysInFirstWeek = 4
    }
    val year = cal.get(Calendar.YEAR)
    val week = cal.get(Calendar.WEEK_OF_YEAR)
    // Basic ISO week logic: if it's week 1 but in December, it might belong to next year.
    // Or if it's late week but in January, it might belong to previous year.
    // However, SimpleDateFormat("YYYYww") with 'Y' (week year) is better.
    return SimpleDateFormat("YYYYww", Locale.US).format(Date())
}

private fun currentMonthStartDateDigits(): String {
    val cal = Calendar.getInstance().apply {
        set(Calendar.DAY_OF_MONTH, 1)
    }
    return SimpleDateFormat("yyyyMMdd", Locale.US).format(cal.time)
}

class QueryReportViewModel(
    private val reportGateway: ReportGateway,
    private val queryGateway: QueryGateway
) : ViewModel() {
    var uiState by mutableStateOf(QueryReportUiState())
        private set

    private val strictBasicIsoDateFormatter: SimpleDateFormat =
        SimpleDateFormat("yyyyMMdd", Locale.US).apply { isLenient = false }
    private val strictMonthDigitsFormatter: SimpleDateFormat =
        SimpleDateFormat("yyyyMM", Locale.US).apply { isLenient = false }
    private val strictIsoYearFormatter: SimpleDateFormat =
        SimpleDateFormat("yyyy", Locale.US).apply { isLenient = false }
    private val periodArgumentResolver = QueryPeriodArgumentResolver()

    private fun digitsOnly(value: String, maxLength: Int): String =
        value.filter { it.isDigit() }.take(maxLength)

    fun onReportDateChange(value: String) {
        uiState = uiState.copy(reportDate = digitsOnly(value, 8))
    }

    fun onReportMonthChange(value: String) {
        uiState = uiState.copy(reportMonth = digitsOnly(value, 6))
    }

    fun onReportYearChange(value: String) {
        uiState = uiState.copy(reportYear = digitsOnly(value, 4))
    }

    fun onReportWeekChange(value: String) {
        uiState = uiState.copy(reportWeek = digitsOnly(value, 6))
    }

    fun onReportRecentDaysChange(value: String) {
        uiState = uiState.copy(reportRecentDays = value.filter { it.isDigit() })
    }

    fun onReportRangeStartDateChange(value: String) {
        uiState = uiState.copy(reportRangeStartDate = digitsOnly(value, 8))
    }

    fun onReportRangeEndDateChange(value: String) {
        uiState = uiState.copy(reportRangeEndDate = digitsOnly(value, 8))
    }

    fun reportDay() {
        viewModelScope.launch {
            val dayDigits = uiState.reportDate.trim()
            val validationError = validateDateDigits(dayDigits)
            if (validationError != null) {
                uiState = uiState.copy(statusText = validationError, activeResult = null)
                return@launch
            }

            val dayIso = toIsoDate(dayDigits)
            uiState = uiState.copy(statusText = "nativeReport(day, md) running...")
            val result = reportGateway.reportDayMarkdown(dayIso)
            uiState = uiState.copy(
                activeResult = if (result.operationOk) QueryResult.Report(result.outputText) else null,
                statusText = "nativeReport(day, md) -> ok=${result.operationOk}"
            )
        }
    }

    fun reportMonth() {
        viewModelScope.launch {
            val monthDigits = uiState.reportMonth.trim()
            val validationError = validateMonthDigits(monthDigits)
            if (validationError != null) {
                uiState = uiState.copy(statusText = validationError, activeResult = null)
                return@launch
            }

            val monthIso = toIsoMonth(monthDigits)
            uiState = uiState.copy(statusText = "nativeReport(month, md) running...")
            val result = reportGateway.reportMonthMarkdown(monthIso)
            uiState = uiState.copy(
                activeResult = if (result.operationOk) QueryResult.Report(result.outputText) else null,
                statusText = "nativeReport(month, md) -> ok=${result.operationOk}"
            )
        }
    }

    fun reportYear() {
        viewModelScope.launch {
            val year = uiState.reportYear.trim()
            val validationError = validateIsoYear(year)
            if (validationError != null) {
                uiState = uiState.copy(statusText = validationError, activeResult = null)
                return@launch
            }

            uiState = uiState.copy(statusText = "nativeReport(year, md) running...")
            val result = reportGateway.reportYearMarkdown(year)
            uiState = uiState.copy(
                activeResult = if (result.operationOk) QueryResult.Report(result.outputText) else null,
                statusText = "nativeReport(year, md) -> ok=${result.operationOk}"
            )
        }
    }

    fun reportWeek() {
        viewModelScope.launch {
            val weekDigits = uiState.reportWeek.trim()
            val validationError = validateWeekDigits(weekDigits)
            if (validationError != null) {
                uiState = uiState.copy(statusText = validationError, activeResult = null)
                return@launch
            }

            val weekIso = toIsoWeek(weekDigits)
            uiState = uiState.copy(statusText = "nativeReport(week, md) running...")
            val result = reportGateway.reportWeekMarkdown(weekIso)
            uiState = uiState.copy(
                activeResult = if (result.operationOk) QueryResult.Report(result.outputText) else null,
                statusText = "nativeReport(week, md) -> ok=${result.operationOk}"
            )
        }
    }

    fun reportRecent() {
        viewModelScope.launch {
            val recentDays = uiState.reportRecentDays.trim()
            val validationError = validateRecentDays(recentDays)
            if (validationError != null) {
                uiState = uiState.copy(statusText = validationError, activeResult = null)
                return@launch
            }

            uiState = uiState.copy(statusText = "nativeReport(recent, md) running...")
            val result = reportGateway.reportRecentMarkdown(recentDays)
            uiState = uiState.copy(
                activeResult = if (result.operationOk) QueryResult.Report(result.outputText) else null,
                statusText = "nativeReport(recent, md) -> ok=${result.operationOk}"
            )
        }
    }

    fun reportRange() {
        viewModelScope.launch {
            val startDateDigits = uiState.reportRangeStartDate.trim()
            val endDateDigits = uiState.reportRangeEndDate.trim()

            val startValidationError = validateDateDigits(startDateDigits)
            if (startValidationError != null) {
                uiState = uiState.copy(
                    statusText = "Range start date invalid. ${startValidationError}",
                    activeResult = null
                )
                return@launch
            }

            val endValidationError = validateDateDigits(endDateDigits)
            if (endValidationError != null) {
                uiState = uiState.copy(
                    statusText = "Range end date invalid. ${endValidationError}",
                    activeResult = null
                )
                return@launch
            }

            val start = strictBasicIsoDateFormatter.parse(startDateDigits)
            val end = strictBasicIsoDateFormatter.parse(endDateDigits)
            if (start != null && end != null && start.after(end)) {
                uiState = uiState.copy(
                    statusText = "Invalid range. Start date must be <= end date.",
                    activeResult = null
                )
                return@launch
            }

            val startIso = SimpleDateFormat("yyyy-MM-dd", Locale.US).format(start!!)
            val endIso = SimpleDateFormat("yyyy-MM-dd", Locale.US).format(end!!)
            uiState = uiState.copy(statusText = "nativeReport(range, md) running...")
            val result = reportGateway.reportRange(startDate = startIso, endDate = endIso)
            uiState = uiState.copy(
                activeResult = if (result.operationOk) QueryResult.Report(result.outputText) else null,
                statusText = "nativeReport(range, md) -> ok=${result.operationOk}"
            )
        }
    }

    fun loadDayStats(period: DataTreePeriod) {
        viewModelScope.launch {
            val resolved = periodArgumentResolver.resolveAndValidate(
                period = period,
                source = currentPeriodSource(),
                subjectLabel = "Stats"
            )
            if (resolved is QueryPeriodResolveResult.Failure) {
                uiState = uiState.copy(
                    analysisLoading = false,
                    analysisError = resolved.message,
                    activeResult = null,
                    statsPeriod = period
                )
                return@launch
            }
            val normalizedArgument = (resolved as QueryPeriodResolveResult.Success).argument

            uiState = uiState.copy(
                analysisLoading = true,
                analysisError = "",
                statsPeriod = period,
                statusText = "query data days-stats running... period=${period.wireValue}"
            )

            val result = queryGateway.queryDayDurationStats(
                DataDurationQueryParams(
                    period = period,
                    periodArgument = normalizedArgument
                )
            )

            uiState = uiState.copy(
                analysisLoading = false,
                analysisError = if (result.ok) "" else result.message,
                activeResult = if (result.ok) QueryResult.Stats(
                    result.outputText,
                    period
                ) else null,
                statsPeriod = period,
                statusText = "query data days-stats(${period.wireValue}) -> ok=${result.ok}"
            )
        }
    }

    fun loadTree(
        period: DataTreePeriod,
        level: Int = -1
    ) {
        viewModelScope.launch {
            uiState = uiState.copy(treePeriod = period)
            val resolved = periodArgumentResolver.resolveAndValidate(
                period = period,
                source = currentPeriodSource(),
                subjectLabel = "Tree"
            )
            if (resolved is QueryPeriodResolveResult.Failure) {
                uiState = uiState.copy(
                    analysisLoading = false,
                    analysisError = resolved.message,
                    activeResult = null
                )
                return@launch
            }
            val normalizedArg = (resolved as QueryPeriodResolveResult.Success).argument
            if (level < -1) {
                uiState = uiState.copy(
                    analysisLoading = false,
                    analysisError = "Tree level must be >= -1.",
                    activeResult = null
                )
                return@launch
            }

            uiState = uiState.copy(
                analysisLoading = true,
                analysisError = "",
                statusText = "query data tree running..."
            )

            val result = queryGateway.queryProjectTree(
                DataTreeQueryParams(
                    period = period,
                    periodArgument = normalizedArg,
                    level = level
                )
            )

            uiState = uiState.copy(
                analysisLoading = false,
                analysisError = if (result.ok) "" else result.message,
                activeResult = if (result.ok) QueryResult.Tree(
                    result.outputText,
                    period
                ) else null,
                statusText = "query data tree -> ok=${result.ok}"
            )
        }
    }

    private fun toIsoDate(value: String): String {
        val date = strictBasicIsoDateFormatter.parse(value)
        return SimpleDateFormat("yyyy-MM-dd", Locale.US).format(date!!)
    }

    private fun validateDateDigits(value: String): String? {
        if (!Regex("""^\d{8}$""").matches(value)) {
            return "Invalid day format. Use digits YYYYMMDD (example: 20260214)."
        }
        return try {
            strictBasicIsoDateFormatter.parse(value)
            null
        } catch (_: Exception) {
            "Invalid date value: $value."
        }
    }

    private fun toIsoMonth(value: String): String {
        val date = strictMonthDigitsFormatter.parse(value)
        return SimpleDateFormat("yyyy-MM", Locale.US).format(date!!)
    }

    private fun validateMonthDigits(value: String): String? {
        if (!Regex("""^\d{6}$""").matches(value)) {
            return "Invalid month format. Use digits YYYYMM (example: 202602)."
        }
        return try {
            strictMonthDigitsFormatter.parse(value)
            null
        } catch (_: Exception) {
            "Invalid month value: $value."
        }
    }

    private fun validateIsoYear(value: String): String? {
        if (!Regex("""^\d{4}$""").matches(value)) {
            return "Invalid year format. Use ISO year: YYYY (example: 2026)."
        }
        return try {
            strictIsoYearFormatter.parse(value)
            null
        } catch (_: Exception) {
            "Invalid ISO year value: $value."
        }
    }

    private fun toIsoWeek(value: String): String {
        val year = value.substring(0, 4).toInt()
        val week = value.substring(4, 6).toInt()
        return String.format(Locale.US, "%04d-W%02d", year, week)
    }

    private fun validateWeekDigits(value: String): String? {
        val match = Regex("""^(\d{4})(\d{2})$""").matchEntire(value)
            ?: return "Invalid week format. Use digits YYYYWW (example: 202607)."

        val year = match.groupValues[1].toIntOrNull()
            ?: return "Invalid week year: ${match.groupValues[1]}."
        val week = match.groupValues[2].toIntOrNull()
            ?: return "Invalid week number: ${match.groupValues[2]}."

        val cal = Calendar.getInstance(Locale.US).apply {
            set(Calendar.YEAR, year)
            set(Calendar.MONTH, Calendar.DECEMBER)
            set(Calendar.DAY_OF_MONTH, 28)
            firstDayOfWeek = Calendar.MONDAY
            minimalDaysInFirstWeek = 4
        }
        val maxIsoWeek = cal.getActualMaximum(Calendar.WEEK_OF_YEAR)
        if (week !in 1..maxIsoWeek) {
            return "Invalid week value: $value. $year supports week 01 to $maxIsoWeek."
        }
        return null
    }

    private fun currentPeriodSource(): QueryPeriodSource = QueryPeriodSource(
        dayDigits = uiState.reportDate,
        monthDigits = uiState.reportMonth,
        yearDigits = uiState.reportYear,
        weekDigits = uiState.reportWeek,
        rangeStartDigits = uiState.reportRangeStartDate,
        rangeEndDigits = uiState.reportRangeEndDate,
        recentDays = uiState.reportRecentDays
    )

    private fun validateRecentDays(value: String): String? {
        if (value.isBlank()) {
            return "Recent days is required and must be a number."
        }
        val days = value.toIntOrNull()
            ?: return "Recent days must be numeric."
        if (days <= 0) {
            return "Recent days must be greater than 0."
        }
        return null
    }
}

class QueryReportViewModelFactory(
    private val reportGateway: ReportGateway,
    private val queryGateway: QueryGateway
) : ViewModelProvider.Factory {
    override fun <T : ViewModel> create(modelClass: Class<T>): T {
        if (modelClass.isAssignableFrom(QueryReportViewModel::class.java)) {
            @Suppress("UNCHECKED_CAST")
            return QueryReportViewModel(reportGateway, queryGateway) as T
        }
        throw IllegalArgumentException("Unknown ViewModel class: ${modelClass.name}")
    }
}
