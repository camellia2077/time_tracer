package com.example.tracer

import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.res.stringResource
import android.util.Log
import com.example.tracer.feature.insights.R
import com.example.tracer.ui.components.CalendarAvailability
import com.example.tracer.ui.components.CalendarYearMonthPickerSheet
import com.example.tracer.ui.components.OutlinedPickerField
import com.example.tracer.ui.components.splitYearMonthDigits

@OptIn(ExperimentalMaterial3Api::class)
@Composable
internal fun InsightsYearMonthPickerInput(
    title: String,
    insightsMonth: String,
    availability: CalendarAvailability,
    valueFormat: InsightsYearMonthValueFormat = InsightsYearMonthValueFormat.COMPACT,
    onInsightsMonthChange: (String) -> Unit
) {
    var visible by remember { mutableStateOf(false) }
    val (year, month) = splitYearMonthDigits(insightsMonth)
    val displayInsightsMonth = if (year.length == 4 && month.length == 2) {
        "$year-$month"
    } else {
        insightsMonth
    }

    val openPicker = {
        Log.d(
            "InsightsCalendar",
            "month picker open selected=$displayInsightsMonth years=${availability.years} " +
                "monthsByYear=${availability.monthsByYear}"
        )
        visible = true
    }
    OutlinedPickerField(
        title = title,
        value = displayInsightsMonth,
        enabled = availability.years.isNotEmpty(),
        onOpen = openPicker
    )

    if (visible) {
        CalendarYearMonthPickerSheet(
            selectedYearMonth = displayInsightsMonth,
            availability = availability,
            onYearMonthSelected = { selected ->
                Log.d("InsightsCalendar", "month picker selected=$selected")
                onInsightsMonthChange(
                    valueFormat.normalize(selected)
                )
            },
            onDismissRequest = { visible = false },
            title = stringResource(R.string.insights_sheet_select_year_month),
            currentText = stringResource(R.string.insights_sheet_current_month, displayInsightsMonth),
            yearTitle = stringResource(R.string.insights_picker_year_title),
            yearPlaceholder = stringResource(R.string.insights_picker_year_placeholder),
            noYearsLabel = stringResource(R.string.insights_picker_no_years)
        )
    }
}

internal enum class InsightsYearMonthValueFormat {
    COMPACT,
    ISO;

    fun normalize(value: String): String = when (this) {
        COMPACT -> value.replace("-", "")
        ISO -> value
    }
}
