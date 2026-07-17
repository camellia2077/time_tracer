package com.example.tracer

import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ArrowDropDown
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import com.example.tracer.feature.report.R
import com.example.tracer.ui.components.CalendarAvailability
import com.example.tracer.ui.components.CalendarYearMonthPickerSheet
import com.example.tracer.ui.components.splitYearMonthDigits

@OptIn(ExperimentalMaterial3Api::class)
@Composable
internal fun ReportYearMonthPickerInput(
    title: String,
    reportMonth: String,
    availability: CalendarAvailability,
    onReportMonthChange: (String) -> Unit
) {
    var visible by remember { mutableStateOf(false) }
    val (year, month) = splitYearMonthDigits(reportMonth)
    val displayReportMonth = if (year.length == 4 && month.length == 2) {
        "$year-$month"
    } else {
        reportMonth
    }

    Text(
        text = title,
        style = MaterialTheme.typography.bodyMedium,
        color = MaterialTheme.colorScheme.onSurfaceVariant
    )
    OutlinedButton(
        onClick = { visible = true },
        enabled = availability.years.isNotEmpty(),
        modifier = Modifier.fillMaxWidth()
    ) {
        Text(
            text = displayReportMonth,
            style = MaterialTheme.typography.titleMedium,
            color = MaterialTheme.colorScheme.primary
        )
        Icon(Icons.Filled.ArrowDropDown, contentDescription = null)
    }

    if (visible) {
        CalendarYearMonthPickerSheet(
            selectedYearMonth = displayReportMonth,
            availability = availability,
            onYearMonthSelected = onReportMonthChange,
            onDismissRequest = { visible = false },
            title = stringResource(R.string.report_sheet_select_year_month),
            currentText = stringResource(R.string.report_sheet_current_month, displayReportMonth),
            yearTitle = stringResource(R.string.report_picker_year_title),
            yearPlaceholder = stringResource(R.string.report_picker_year_placeholder),
            noYearsLabel = stringResource(R.string.report_picker_no_years)
        )
    }
}
