package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.unit.dp

enum class InsightsMode {
    DAY,
    WEEK,
    MONTH,
    YEAR,
    RANGE,
    RECENT
}

@Composable
internal fun QueryInsightsSection(
    showModeTabs: Boolean = true,
    insightsMode: InsightsMode,
    onInsightsModeChange: (InsightsMode) -> Unit,
    insightsDate: String,
    onInsightsDateChange: (String) -> Unit,
    insightsMonth: String,
    onInsightsMonthChange: (String) -> Unit,
    calendarAvailability: com.example.tracer.ui.components.CalendarAvailability,
    insightsYear: String,
    onInsightsYearChange: (String) -> Unit,
    insightsWeek: String,
    onInsightsWeekChange: (String) -> Unit,
    insightsRangeStartDate: String,
    onInsightsRangeStartDateChange: (String) -> Unit,
    insightsRangeEndDate: String,
    onInsightsRangeEndDateChange: (String) -> Unit,
    insightsRecentDays: String,
    onInsightsRecentDaysChange: (String) -> Unit,
    onInsightsActivityPeriodConfirmed: (InsightsPeriodSelection) -> Unit = {},
    resultDisplayMode: InsightsResultDisplayMode,
    onResultDisplayModeChange: (InsightsResultDisplayMode) -> Unit,
    chartSemanticMode: InsightsChartSemanticMode,
    onChartSemanticModeChange: (InsightsChartSemanticMode) -> Unit,
    selectedParameterSection: InsightsParameterSection,
    treeLevel: Int,
    treeMaxAvailableDepth: Int,
    onSelectedParameterSectionChange: (InsightsParameterSection) -> Unit,
    onTreeLevelChange: (Int) -> Unit,
    timeParametersExpanded: Boolean,
    onTimeParametersExpandedChange: (Boolean) -> Unit
) {
    val analysisPeriod = insightsMode.toDataTreePeriod()
    val insightsModes = InsightsMode.entries
    val numericKeyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Number)

    Column(
        modifier = Modifier.fillMaxWidth(),
        verticalArrangement = Arrangement.spacedBy(16.dp)
    ) {
        if (showModeTabs) {
            InsightsModeTabs(
                insightsModes = insightsModes,
                insightsMode = insightsMode,
                onInsightsModeChange = onInsightsModeChange
            )
        }
        Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
            InsightsResultModeSwitcher(
                mode = resultDisplayMode,
                onModeChange = onResultDisplayModeChange,
                modifier = Modifier.fillMaxWidth()
            )
            if (resultDisplayMode == InsightsResultDisplayMode.CHART &&
                insightsMode != InsightsMode.DAY
            ) {
                InsightsChartSemanticModeSelector(
                    chartSemanticMode = chartSemanticMode,
                    onChartSemanticModeChange = onChartSemanticModeChange
                )
            }

            QueryInsightsParameterCards(
                insightsMode = insightsMode,
                resultDisplayMode = resultDisplayMode,
                analysisPeriod = analysisPeriod,
                selectedSection = selectedParameterSection,
                treeMaxAvailableDepth = treeMaxAvailableDepth,
                treeLevel = treeLevel.toString(),
                keyboardOptions = numericKeyboardOptions,
                insightsDate = insightsDate,
                onInsightsDateChange = onInsightsDateChange,
                insightsMonth = insightsMonth,
                onInsightsMonthChange = onInsightsMonthChange,
                calendarAvailability = calendarAvailability,
                insightsYear = insightsYear,
                onInsightsYearChange = onInsightsYearChange,
                insightsWeek = insightsWeek,
                onInsightsWeekChange = onInsightsWeekChange,
                insightsRangeStartDate = insightsRangeStartDate,
                onInsightsRangeStartDateChange = onInsightsRangeStartDateChange,
                insightsRangeEndDate = insightsRangeEndDate,
                onInsightsRangeEndDateChange = onInsightsRangeEndDateChange,
                insightsRecentDays = insightsRecentDays,
                onInsightsRecentDaysChange = onInsightsRecentDaysChange,
                onInsightsActivityPeriodConfirmed = onInsightsActivityPeriodConfirmed,
                timeParametersExpanded = timeParametersExpanded,
                onTimeParametersExpandedChange = onTimeParametersExpandedChange,
                onSelectedSectionChange = onSelectedParameterSectionChange,
                onTreeLevelChange = {
                    val normalizedLevel = normalizeSignedIntInput(it, 3)
                    onTreeLevelChange(normalizedLevel.toIntOrNull() ?: -1)
                }
            )
        }
    }
}

@Composable
internal fun InsightsModeTabs(
    insightsModes: List<InsightsMode>,
    insightsMode: InsightsMode,
    onInsightsModeChange: (InsightsMode) -> Unit
) {
    StaticScrollableTextTabRow(
        labels = insightsModes.map { stringResource(it.labelRes()) },
        selectedIndex = insightsModes.indexOf(insightsMode),
        onSelectedIndexChange = { index -> onInsightsModeChange(insightsModes[index]) }
    )
}

internal fun InsightsMode.toDataTreePeriod(): DataTreePeriod =
    when (this) {
        InsightsMode.DAY -> DataTreePeriod.DAY
        InsightsMode.WEEK -> DataTreePeriod.WEEK
        InsightsMode.MONTH -> DataTreePeriod.MONTH
        InsightsMode.YEAR -> DataTreePeriod.YEAR
        InsightsMode.RANGE -> DataTreePeriod.RANGE
        InsightsMode.RECENT -> DataTreePeriod.RECENT
    }

private fun normalizeSignedIntInput(value: String, maxDigits: Int): String {
    if (value.isEmpty()) {
        return ""
    }
    val trimmed = value.trim()
    if (trimmed == "-") {
        return "-"
    }
    val isNegative = trimmed.startsWith("-")
    val digits = trimmed.filter { it.isDigit() }.take(maxDigits)
    return if (isNegative) "-$digits" else digits
}
